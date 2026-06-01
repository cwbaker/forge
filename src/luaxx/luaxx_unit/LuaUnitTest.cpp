
#define _CRT_SECURE_NO_DEPRECATE

#include "LuaUnitTest.hpp"
#include <error/ErrorPolicy.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <UnitTest++/TestDetails.h>
#include <UnitTest++/TestResults.h>
#include <UnitTest++/TestReporter.h>
#include <UnitTest++/Checks.h>
#include <UnitTest++/MemoryOutStream.h>
#include <UnitTest++/CurrentTest.h>
#include <UnitTest++/UnitTest++.h>
#include <string>
#include <stdio.h>

#if defined(BUILD_OS_WINDOWS)
#include <UnitTest++/Win32/TimeHelpers.h>
#else
#include <UnitTest++/Posix/TimeHelpers.h>
#endif

using std::string;
using namespace UnitTest;
using namespace sweet;
using namespace luaxx;

#if defined(BUILD_OS_WINDOWS)
#define snprintf _snprintf
#endif

LuaUnitTest::LuaUnitTest()
: lua_state_( nullptr )
, error_policy_( nullptr )
, running_suite_name_()
, running_test_name_()
, running_test_file_()
, previous_details_( nullptr )
, details_( nullptr )
, timer_( nullptr )
{
}

LuaUnitTest::LuaUnitTest( lua_State* lua_state, error::ErrorPolicy* error_policy )
: lua_state_( nullptr )
, error_policy_( nullptr )
, running_suite_name_()
, running_test_name_()
, running_test_file_()
, previous_details_( nullptr )
, details_( nullptr )
, timer_( nullptr )
{
    create( lua_state, error_policy );
}

LuaUnitTest::~LuaUnitTest()
{
    delete timer_;
    delete details_;
}

void LuaUnitTest::create( lua_State* lua_state, sweet::error::ErrorPolicy* error_policy )
{
    SWEET_ASSERT( lua_state );

    lua_state_ = lua_state;
    error_policy_ = error_policy;

    // Thin C functions that bracket each test with UnitTest++ bookkeeping.
    // The `this` pointer is passed as a shared upvalue so that the start and
    // finish calls can persist the running test's details and timer.  See the
    // class comment in the header for why the driver itself is Lua.
    static const luaL_Reg functions[] =
    {
        { "start_test", &LuaUnitTest::start_test },
        { "finish_test", &LuaUnitTest::finish_test },
        { "fail_test", &LuaUnitTest::fail_test },
        { "CHECK", &LuaUnitTest::check },
        { "CHECK_EQUAL", &LuaUnitTest::check_equal },
        { "CHECK_CLOSE", &LuaUnitTest::check_close },
        { nullptr, nullptr }
    };
    lua_pushglobaltable( lua_state_ );
    lua_pushlightuserdata( lua_state_, this );
    luaL_setfuncs( lua_state_, functions, 1 );
    lua_pop( lua_state_, 1 );

    // The `SUITE()` / `TEST()` / `TEST_FIXTURE()` driver.  `SUITE()`
    // records the current suite name; `TEST()` and `TEST_FIXTURE()` each run a
    // single test immediately, in source order, through the yieldable Lua
    // `xpcall()`.  The `fixture` argument to `TEST_FIXTURE()` is a function
    // that is called before the test; its return values (zero or more) are
    // forwarded as arguments to the test.  This lets a fixture both establish
    // per-test state (e.g. reset graph and error state) and hand objects to
    // the test; fixtures expose such functions to scripts as, for example,
    // `_unit_test_setup`.
    static const char DRIVER [] =
        "local current_suite = nil\n"
        "function SUITE( name )\n"
        "    current_suite = name;\n"
        "end\n"
        "local function run_test( name, test, arguments )\n"
        "    start_test( current_suite, name, test );\n"
        "    local ok, message = xpcall( test, debug.traceback, table.unpack(arguments, 1, arguments.n) );\n"
        "    if not ok then\n"
        "        fail_test( message );\n"
        "    end\n"
        "    finish_test();\n"
        "end\n"
        "function TEST( name, test )\n"
        "    run_test( name, test, table.pack() );\n"
        "end\n"
        "function TEST_FIXTURE( fixture, name, test )\n"
        "    run_test( name, test, table.pack(fixture()) );\n"
        "end\n";

    bool failed = luaL_dostring( lua_state_, DRIVER ) != LUA_OK;
    if ( failed )
    {
        if ( error_policy_ )
        {
            error_policy_->error( failed, "Defining the unit test driver failed - %s", lua_tostring(lua_state_, -1) );
        }
        lua_pop( lua_state_, 1 );
    }
}

void LuaUnitTest::run_tests( const char* filename )
{
    SWEET_ASSERT( filename );
    bool failed = luaL_dofile( lua_state_, filename );
    error_policy_->error( failed, "Executing '%s' failed", filename );
}

int LuaUnitTest::start_test( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( CurrentTest::Details() );
    SWEET_ASSERT( CurrentTest::Results() );

    LuaUnitTest* self = (LuaUnitTest*) lua_touserdata( lua_state, lua_upvalueindex(1) );
    SWEET_ASSERT( self );
    SWEET_ASSERT( !self->details_ );

    const int SUITE = 1;
    const int NAME = 2;
    const int FUNCTION = 3;

    // The suite name comes from `SUITE()`; fall back to the enclosing
    // (C++) suite when a script hasn't named one.
    const UnitTest::TestDetails* current_details = UnitTest::CurrentTest::Details();
    self->running_suite_name_ = luaL_optstring( lua_state, SUITE, current_details->suiteName );
    self->running_test_name_ = luaL_checkstring( lua_state, NAME );

    // Recover the source file and line that the test function is defined on so
    // that failures are reported against the test rather than this binding.
    luaL_checktype( lua_state, FUNCTION, LUA_TFUNCTION );
    lua_pushvalue( lua_state, FUNCTION );
    lua_Debug debug;
    lua_getinfo( lua_state, ">S", &debug );
    self->running_test_file_ = debug.source;
    int line = debug.linedefined;

    self->previous_details_ = current_details;
    self->details_ = new UnitTest::TestDetails(
        self->running_test_name_.c_str(),
        self->running_suite_name_.c_str(),
        self->running_test_file_.c_str(),
        line
    );
    UnitTest::CurrentTest::Details() = self->details_;

    self->timer_ = new Timer;
    self->timer_->Start();
    CurrentTest::Results()->OnTestStart( *self->details_ );
    return 0;
}

int LuaUnitTest::fail_test( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( CurrentTest::Details() );
    SWEET_ASSERT( CurrentTest::Results() );

    const int MESSAGE = 1;
    const char* message = luaL_optstring( lua_state, MESSAGE, "Test failed with an unknown error" );
    CurrentTest::Results()->OnTestFailure( *CurrentTest::Details(), message );
    return 0;
}

int LuaUnitTest::finish_test( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( CurrentTest::Results() );

    LuaUnitTest* self = (LuaUnitTest*) lua_touserdata( lua_state, lua_upvalueindex(1) );
    SWEET_ASSERT( self );
    SWEET_ASSERT( self->details_ );
    SWEET_ASSERT( self->timer_ );

    const double MAXIMUM_TIME = 1000.0;
    const double time = self->timer_->GetTimeInMs();
    if ( time > MAXIMUM_TIME )
    {
        MemoryOutStream stream;
        stream << "Global time constraint failed. Expected under " << MAXIMUM_TIME << "ms but took " << time << "ms.";
        CurrentTest::Results()->OnTestFailure( *self->details_, stream.GetText() );
    }
    CurrentTest::Results()->OnTestFinish( *self->details_, static_cast<float>(time) / 1000.0f );

    CurrentTest::Details() = self->previous_details_;
    self->previous_details_ = nullptr;
    delete self->details_;
    self->details_ = nullptr;
    delete self->timer_;
    self->timer_ = nullptr;

    lua_gc( lua_state, LUA_GCCOLLECT, 0 );
    lua_gc( lua_state, LUA_GCCOLLECT, 0 );
    return 0;
}

int LuaUnitTest::check( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int EXPRESSION = 1;
    const int MESSAGE = 2;

    bool expression = lua_toboolean( lua_state, EXPRESSION );
    if ( !expression )
    {
        string message = lua_isstring( lua_state, MESSAGE ) ? lua_tostring( lua_state, MESSAGE ) : string();
        UnitTest::CurrentTest::Results()->OnTestFailure( UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)), message.c_str() );
    }

    return 0;
}

int LuaUnitTest::check_equal( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int EXPECTED = 1;
    const int ACTUAL = 2;

    switch ( lua_type(lua_state, 1) )
    {
        case LUA_TBOOLEAN:
        {
            bool expected = lua_toboolean( lua_state, EXPECTED );
            bool actual = lua_toboolean( lua_state, ACTUAL );
            UnitTest::CheckEqual( *UnitTest::CurrentTest::Results(), expected, actual, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)) );
            break;
        }

        case LUA_TNUMBER:
        {
            lua_Integer expected = luaL_checkinteger( lua_state, EXPECTED );
            lua_Integer actual = luaL_checkinteger( lua_state, ACTUAL );
            UnitTest::CheckEqual( *UnitTest::CurrentTest::Results(), expected, actual, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)) );
            break;
        }

        case LUA_TSTRING:
        {
            const char* expected = lua_tostring( lua_state, EXPECTED );
            const char* actual = lua_tostring( lua_state, ACTUAL );
            UnitTest::CheckEqual( *UnitTest::CurrentTest::Results(), expected, actual, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)) );
            break;
        }

        case LUA_TNIL:
        case LUA_TLIGHTUSERDATA:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
            UnitTest::CurrentTest::Results()->OnTestFailure( UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)), "CHECK_EQUAL() is only valid on booleans, numbers, and strings" );
            break;

        default:
        {
            SWEET_ASSERT( false );
            char message [256];
            snprintf( message, sizeof(message), "CHECK_EQUAL() passed an unexpected type, type=%d '%s'", lua_type(lua_state, EXPECTED), lua_typename(lua_state, lua_type(lua_state, EXPECTED)) );
            UnitTest::CurrentTest::Results()->OnTestFailure( UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)), message );
            break;
        }
    }
    return 0;
}

int LuaUnitTest::check_close( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int EXPECTED = 1;
    const int ACTUAL = 2;
    const int TOLERANCE = 3;

    switch ( lua_type(lua_state, 1) )
    {
        case LUA_TNUMBER:
        {
            float expected = static_cast<float>( lua_tonumber( lua_state, EXPECTED) );
            float actual = static_cast<float>( lua_tonumber( lua_state, ACTUAL) );
            float tolerance = static_cast<float>( lua_tonumber(lua_state, TOLERANCE) );
            UnitTest::CheckClose( *UnitTest::CurrentTest::Results(), expected, actual, tolerance, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)) );
            break;
        }

        case LUA_TNIL:
        case LUA_TLIGHTUSERDATA:
        case LUA_TBOOLEAN:
        case LUA_TSTRING:
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TUSERDATA:
        case LUA_TTHREAD:
            UnitTest::CurrentTest::Results()->OnTestFailure( UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)), "CHECK_CLOSE() is only valid on numbers" );
            break;

        default:
            SWEET_ASSERT( false );
            char message [256];
            snprintf( message, sizeof(message), "CHECK_CLOSE() passed an unexpected type, type=%d '%s'", lua_type(lua_state, EXPECTED), lua_typename(lua_state, lua_type(lua_state, EXPECTED)) );
            UnitTest::CurrentTest::Results()->OnTestFailure( UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), LuaUnitTest::line(lua_state)), message );
            break;
    }

    return 0;
}

int LuaUnitTest::line( lua_State* lua_state )
{
    lua_Debug debug;
    lua_getstack( lua_state, 1, &debug );
    lua_getinfo( lua_state, "l", &debug );
    return debug.currentline;
}

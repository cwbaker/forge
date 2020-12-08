
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

LuaUnitTest::LuaUnitTest( lua_State* lua_state, error::ErrorPolicy* error_policy )
: lua_state_( lua_state )
, error_policy_( error_policy )
{
    SWEET_ASSERT( lua_state_ );

    static const luaL_Reg functions[] = 
    {
        { "TestSuite", &LuaUnitTest::test_suite },
        { "CHECK", &LuaUnitTest::check },
        { "CHECK_EQUAL", &LuaUnitTest::check_equal },
        { "CHECK_CLOSE", &LuaUnitTest::check_close },
        { nullptr, nullptr }
    };
    lua_pushglobaltable( lua_state_ );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_pop( lua_state_, 1 );
}

void LuaUnitTest::run_tests( const char* filename )
{
    SWEET_ASSERT( filename );
    bool failed = luaL_dofile( lua_state_, filename );
    error_policy_->error( failed, "Executing '%s' failed", filename );
}

int LuaUnitTest::test_suite( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( CurrentTest::Details() );
    SWEET_ASSERT( CurrentTest::Results() );
    
    const int TEST_SUITE = 1;
    
    luaL_checktype( lua_state, TEST_SUITE, LUA_TTABLE );

    lua_pushnil( lua_state );
    while ( lua_next(lua_state, TEST_SUITE) != 0 )
    {
        if ( lua_isfunction(lua_state, -1) && lua_isstring(lua_state, -2) )
        {
            lua_pushvalue( lua_state, -1 );
            lua_Debug debug;
            lua_getinfo( lua_state, ">S", &debug );
            string file = debug.source;
            int line = debug.linedefined;

            string test_name = lua_tostring( lua_state, -2 );
	        const UnitTest::TestDetails* current_details = UnitTest::CurrentTest::Details();
	        UnitTest::TestDetails details( test_name.c_str(), current_details->suiteName, file.c_str(), line );
	        UnitTest::CurrentTest::Details() = &details;

	        Timer test_timer;
	        test_timer.Start();
	        CurrentTest::Results()->OnTestStart( details );
            
            int result = lua_pcall( lua_state, 0, 0, 0 );
            switch ( result )
            {
                case 0:
                    break;

                case LUA_ERRRUN:
                {
                    MemoryOutStream stream;
                    stream << lua_tostring( lua_state, -1 );
                    CurrentTest::Results()->OnTestFailure( details, stream.GetText() );
                    lua_pop( lua_state, 1 );
                    break;
                }

                case LUA_ERRMEM:
                {
                    MemoryOutStream stream;
                    stream << "Out of memory - " << lua_tostring( lua_state, -1 );
                    CurrentTest::Results()->OnTestFailure( details, stream.GetText() );
                    lua_pop( lua_state, 1 );
                    break;
                }

                case LUA_ERRERR:
                {
                    MemoryOutStream stream;
                    stream << "Error handler failed - " << lua_tostring( lua_state, -1 );
                    CurrentTest::Results()->OnTestFailure( details, stream.GetText() );                    
                    lua_pop( lua_state, 1 );
                    break;
                }

                case -1:
                {
                    MemoryOutStream stream;
                    stream << "Execution failed due to an unhandled C++ exception";
                    CurrentTest::Results()->OnTestFailure( details, stream.GetText() );
                    break;
                }

                default:
                {
                    SWEET_ASSERT( false );
                    MemoryOutStream stream;
                    stream << "Execution failed in an unexpected way - " << lua_tostring( lua_state, -1 );
                    CurrentTest::Results()->OnTestFailure( details, stream.GetText() );
                    lua_pop( lua_state, 1 );
                    break;
                }
            }

            const double MAXIMUM_TIME = 1000.0;
	        const double time = test_timer.GetTimeInMs();
	        if ( time > MAXIMUM_TIME )
	        {
	            MemoryOutStream stream;
	            stream << "Global time constraint failed. Expected under " << MAXIMUM_TIME << "ms but took " << time << "ms.";
	            CurrentTest::Results()->OnTestFailure( details, stream.GetText() );
	        }
	        CurrentTest::Results()->OnTestFinish( details, static_cast<float>(time) / 1000.0f );
	        CurrentTest::Details() = current_details;
            lua_gc( lua_state, LUA_GCCOLLECT, 0 );
            lua_gc( lua_state, LUA_GCCOLLECT, 0 );
        }
        else
        {
            lua_pop( lua_state, 1 );
        }
    }

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

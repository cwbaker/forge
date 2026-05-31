//
// ForgeLuaFixture.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include "FileFixture.hpp"
#include "ErrorFixture.hpp"
#include <forge/Forge.hpp>
#include <forge/Graph.hpp>
#include <forge/Context.hpp>
#include <forge/Target.hpp>
#include <luaxx/luaxx_unit/LuaUnitTest.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestDetails.h>
#include <UnitTest++/TestResults.h>
#include <UnitTest++/TestReporter.h>
#include <UnitTest++/Checks.h>
#include <UnitTest++/MemoryOutStream.h>
#include <UnitTest++/CurrentTest.h>
#include <filesystem>
#include <string>
#include <string.h>

#if defined(BUILD_OS_WINDOWS)
#include <UnitTest++/Win32/TimeHelpers.h>
#else
#include <UnitTest++/Posix/TimeHelpers.h>
#endif

using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::forge;
using namespace luaxx;
using namespace UnitTest;
using namespace std::filesystem;

ForgeLuaFixture::ForgeLuaFixture()
: forge( nullptr )
, lua_unit_test( nullptr )
, file_fixture( nullptr )
, error_fixture( nullptr )
{
    error_fixture = new ErrorFixture;
    path path = std::filesystem::path( string(TEST_DIRECTORY) );
    forge = new forge::Forge( path.string(), *error_fixture );
    lua_unit_test = new LuaUnitTest;
    file_fixture = new FileFixture;

    forge->reset();
    forge->set_root_directory( path.generic_string() );
    forge->set_stack_trace_enabled( true );
    forge->set_package_path(
        forge->root("../lua/?.lua").generic_string() + ";" +
        forge->root("../lua/?/init.lua").generic_string()
    );

    lua_unit_test->create( forge->lua_state(), error_fixture );

    static const luaL_Reg file_functions [] =
    {
        { "TestSuite", &ForgeLuaFixture::test_suite },
        { "quiet", &ForgeLuaFixture::quiet },
        { "verbose", &ForgeLuaFixture::verbose },
        { "create", &ForgeLuaFixture::create },
        { "remove", &ForgeLuaFixture::remove },
        { "touch", &ForgeLuaFixture::touch },
        { "error_message", &ForgeLuaFixture::error_message },
        { nullptr, nullptr }
    };

    lua_State* lua_state = forge->lua_state();
    lua_pushglobaltable( lua_state );
    lua_pushlightuserdata( lua_state, this );
    luaL_setfuncs( lua_state, file_functions, 1 );
    lua_pop( lua_state, 1 );
}

ForgeLuaFixture::~ForgeLuaFixture()
{
    delete file_fixture;
    delete lua_unit_test;
    delete forge;
    delete error_fixture;
}

int ForgeLuaFixture::test_suite( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( CurrentTest::Details() );
    SWEET_ASSERT( CurrentTest::Results() );

    const int FIXTURE = lua_upvalueindex( 1 );
    const int TEST_SUITE = 1;

    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    Forge* forge = fixture->forge;
    ErrorFixture* error_fixture = fixture->error_fixture;
    SWEET_ASSERT( forge );
    SWEET_ASSERT( error_fixture );

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

            Context* context = forge->context();
            string working_directory = context->working_directory()->path();
            forge->graph()->clear();
            context->reset_directory( working_directory );
            error_fixture->clear();

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

int ForgeLuaFixture::quiet( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    ErrorFixture* error_fixture = fixture->error_fixture;
    SWEET_ASSERT( error_fixture );
    error_fixture->quiet();
    return 0;
}

int ForgeLuaFixture::verbose( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    ErrorFixture* error_fixture = fixture->error_fixture;
    SWEET_ASSERT( error_fixture );
    error_fixture->verbose();
    return 0;
}

int ForgeLuaFixture::error_message( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int INDEX = 1;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    ErrorFixture* error_fixture = fixture->error_fixture;
    SWEET_ASSERT( error_fixture );
    int index = (int) luaL_checkinteger( lua_state, INDEX );
    const char* message = error_fixture->error_message( index );
    lua_pushlstring( lua_state, message, strlen(message) );
    return 1;
}

int ForgeLuaFixture::create( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    const int TIMESTAMP = 2;
    const int CONTENT = 3;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    FileFixture* file_fixture = (FileFixture*) fixture->file_fixture;
    Forge* forge = fixture->forge;
    std::time_t timestamp = luaL_optinteger( lua_state, TIMESTAMP, 0 );
    const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
    const char* content = luaL_optstring( lua_state, CONTENT, "" );
    file_fixture->create( forge->root(filename).string().c_str(), content, timestamp );
    return 0;
}

int ForgeLuaFixture::remove( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    FileFixture* file_fixture = (FileFixture*) fixture->file_fixture;
    Forge* forge = fixture->forge;
    const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
    file_fixture->remove( forge->root(filename).string().c_str() );
    return 0;
}

int ForgeLuaFixture::touch( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    const int TIMESTAMP = 2;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    FileFixture* file_fixture = (FileFixture*) fixture->file_fixture;
    Forge* forge = fixture->forge;
    std::time_t timestamp = luaL_optinteger( lua_state, TIMESTAMP, 0 );
    const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
    file_fixture->touch( forge->root(filename).string().c_str(), timestamp );
    return 0;
}

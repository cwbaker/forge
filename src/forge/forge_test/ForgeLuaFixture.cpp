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
        { "_unit_test_setup", &ForgeLuaFixture::unit_test_setup },
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

int ForgeLuaFixture::unit_test_setup( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int FIXTURE = lua_upvalueindex( 1 );

    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    Forge* forge = fixture->forge;
    ErrorFixture* error_fixture = fixture->error_fixture;
    SWEET_ASSERT( forge );
    SWEET_ASSERT( error_fixture );

    // Reset graph, working directory, and recorded errors before each test so
    // that tests run independently of one another.  Invoked by the Lua
    // unit test driver (see `LuaUnitTest`) ahead of each test function.
    Context* context = forge->context();
    string working_directory = context->working_directory()->path();
    forge->graph()->clear();
    context->reset_directory( working_directory );
    error_fixture->clear();
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

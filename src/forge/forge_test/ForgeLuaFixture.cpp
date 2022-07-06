//
// ForgeLuaFixture.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include "FileChecker.hpp"
#include <forge/Forge.hpp>
#include <error/ErrorPolicy.hpp>
#include <luaxx/luaxx_unit/LuaUnitTest.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <UnitTest++/UnitTest++.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>

using std::string;
using namespace sweet;
using namespace sweet::forge;
using namespace luaxx;
using namespace boost::filesystem;

ForgeLuaFixture::ForgeLuaFixture()
: forge( nullptr )
, lua_unit_test( nullptr )
, file_checker( nullptr )
, show_errors( false )
{
    path path = boost::filesystem::path( string(TEST_DIRECTORY) );
    forge = new forge::Forge( path.string(), *this, this );
    lua_unit_test = new LuaUnitTest;
    file_checker = new FileChecker;

    forge->reset();
    forge->set_root_directory( path.generic_string() );
    forge->set_stack_trace_enabled( true );
    forge->set_package_path(
        forge->root("../lua/?.lua").generic_string() + ";" +
        forge->root("../lua/?/init.lua").generic_string()
    );

    lua_unit_test->create( forge->lua_state(), this );

    static const luaL_Reg file_functions [] = 
    {
        { "quiet", &ForgeLuaFixture::quiet },
        { "verbose", &ForgeLuaFixture::verbose },
        { "create", &ForgeLuaFixture::create },
        { "remove", &ForgeLuaFixture::remove },
        { "touch", &ForgeLuaFixture::touch },
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
    delete file_checker;
    delete lua_unit_test;
    delete forge;
}

void ForgeLuaFixture::report_error( const char* message )
{
    SWEET_ASSERT( message );
    if ( show_errors )
    {
        fputs( message, stderr );
        fputs( "\n", stderr );
        fflush( stderr );
    }
}

void ForgeLuaFixture::report_print( const char* message )
{
    SWEET_ASSERT( message );
    if ( show_errors )
    {
        fputs( message, stdout );
        fputs( "\n", stdout );
        fflush( stdout );
    }
}

void ForgeLuaFixture::forge_output( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    if ( show_errors )
    {
        fputs( message, stdout );
        fputs( "\n", stdout );
        fflush( stdout );
    }
}

void ForgeLuaFixture::forge_warning( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );   
    if ( show_errors )
    {
        fputs( "forge_test: ", stderr );
        fputs( message, stderr );
        fputs( ".\n", stderr );
        fflush( stderr );
    }        
}

void ForgeLuaFixture::forge_error( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    if ( show_errors )
    {
        fputs( "forge_test: ", stderr );
        fputs( message, stderr );
        fputs( ".\n", stderr );
        fflush( stderr );
    }   
}

int ForgeLuaFixture::quiet( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    fixture->show_errors = false;
    return 0;
}

int ForgeLuaFixture::verbose( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    SWEET_ASSERT( fixture );
    fixture->show_errors = true;
    return 0;
}

int ForgeLuaFixture::create( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    const int TIMESTAMP = 2;
    const int CONTENT = 3;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    FileChecker* file_checker = (FileChecker*) fixture->file_checker;
    Forge* forge = fixture->forge;
    std::time_t timestamp = luaL_optinteger( lua_state, TIMESTAMP, 0 );
    const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
    const char* content = luaL_optstring( lua_state, CONTENT, "" );
    file_checker->create( forge->root(filename).string().c_str(), content, timestamp );
    return 0;
}

int ForgeLuaFixture::remove( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    FileChecker* file_checker = (FileChecker*) fixture->file_checker;
    Forge* forge = fixture->forge;
    const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
    file_checker->remove( forge->root(filename).string().c_str() );
    return 0;
}

int ForgeLuaFixture::touch( lua_State* lua_state )
{
    const int FIXTURE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    const int TIMESTAMP = 2;
    ForgeLuaFixture* fixture = (ForgeLuaFixture*) lua_touserdata( lua_state, FIXTURE );
    FileChecker* file_checker = (FileChecker*) fixture->file_checker;
    Forge* forge = fixture->forge;
    std::time_t timestamp = luaL_optinteger( lua_state, TIMESTAMP, 0 );
    const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
    file_checker->touch( forge->root(filename).string().c_str(), timestamp );
    return 0;
}

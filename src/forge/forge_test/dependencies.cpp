//
// TestTransitiveDependencies.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include "FileChecker.hpp"
#include <forge/Forge.hpp>
#include <forge/ForgeEventSink.hpp>
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

SUITE( dependencies )
{
    struct LuaTest : public forge::ForgeEventSink
    {
        error::ErrorPolicy* error_policy_;
        forge::Forge* forge_;
        luaxx::LuaUnitTest* lua_unit_test_;
        FileChecker* file_checker_;
    
        LuaTest()
        {
            path path = boost::filesystem::path( string(TEST_DIRECTORY) );
            error_policy_ = new error::ErrorPolicy;
            forge_ = new forge::Forge( path.string(), *error_policy_, this );
            lua_unit_test_ = new LuaUnitTest( forge_->lua_state(), error_policy_ );
            file_checker_ = new FileChecker;

            forge_->set_root_directory( path.generic_string() );
            forge_->set_stack_trace_enabled( true );
            forge_->set_package_path(
                forge_->root("../lua/?.lua").generic_string() + ";" +
                forge_->root("../lua/?/init.lua").generic_string()
            );

            static const luaL_Reg file_functions [] = 
            {
                { "create", &LuaTest::create },
                { "remove", &LuaTest::remove },
                { "touch", &LuaTest::touch },
                { nullptr, nullptr }
            };
            lua_State* lua_state = forge_->lua_state();
            lua_pushglobaltable( lua_state );
            lua_pushlightuserdata( lua_state, file_checker_ );
            luaL_setfuncs( lua_state, file_functions, 1 );    
            lua_pop( lua_state, 1 );
        }

        ~LuaTest()
        {
            delete file_checker_;
            delete lua_unit_test_;
            delete forge_;
            delete error_policy_;
        }

        void forge_output( Forge* /*forge*/, const char* message )
        {
            SWEET_ASSERT( message );
            fputs( message, stdout );
            fputs( "\n", stdout );
            fflush( stdout );
        }

        void forge_warning( Forge* /*forge*/, const char* message )
        {
            SWEET_ASSERT( message );            
            fputs( "forge_test: ", stderr );
            fputs( message, stderr );
            fputs( ".\n", stderr );
            fflush( stderr );
        }

        void forge_error( Forge* /*forge*/, const char* message )
        {
            SWEET_ASSERT( message );            
            fputs( "forge_test: ", stderr );
            fputs( message, stderr );
            fputs( ".\n", stderr );
            fflush( stderr );
        }

        static int create( lua_State* lua_state )
        {
            const int LUA_TEST = lua_upvalueindex( 1 );
            const int FILENAME = 1;
            const int TIMESTAMP = 2;
            const int CONTENT = 3;
            FileChecker* file_checker = (FileChecker*) lua_touserdata( lua_state, LUA_TEST );
            std::time_t timestamp = luaL_optinteger( lua_state, TIMESTAMP, 0 );
            const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
            const char* content = luaL_optstring( lua_state, CONTENT, "" );
            file_checker->create( filename, content, timestamp );
            return 0;
        }

        static int remove( lua_State* lua_state )
        {
            const int LUA_TEST = lua_upvalueindex( 1 );
            const int FILENAME = 1;
            FileChecker* file_checker = (FileChecker*) lua_touserdata( lua_state, LUA_TEST );
            const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
            file_checker->remove( filename );
            return 0;
        }

        static int touch( lua_State* lua_state )
        {
            const int LUA_TEST = lua_upvalueindex( 1 );
            const int FILENAME = 1;
            const int TIMESTAMP = 2;
            FileChecker* file_checker = (FileChecker*) lua_touserdata( lua_state, LUA_TEST );
            std::time_t timestamp = luaL_optinteger( lua_state, TIMESTAMP, 0 );
            const char* filename = luaL_tolstring( lua_state, FILENAME, nullptr );
            file_checker->touch( filename, timestamp );
            return 0;
        }

    };

    TEST_FIXTURE( LuaTest, transitive_dependencies )
    {
        forge_->file( "transitive_dependencies.lua" );
    }
}

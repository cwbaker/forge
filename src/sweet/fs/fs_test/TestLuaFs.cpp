//
// TestLuaFs.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include <unit/UnitTest.h>
#include <sweet/fs/fs.hpp>
#include <sweet/fs/FileSystem.hpp>
#include <sweet/fs/DirectoryStack.hpp>
#include <sweet/fs/fs_lua/LuaFs.hpp>
#include <lua/lua.hpp>

using namespace sweet::fs;

SUITE( TestLuaFs )
{
    TEST( change_directory )
    {
        lua_State* lua_state = luaL_newstate();
        luaL_openlibs( lua_state );

        DirectoryStack directory_stack;
        FileSystem file_system( &directory_stack );
        LuaFs lua_fs;
        lua_newtable( lua_state );
        lua_fs.create( &file_system, lua_state );
        lua_setglobal( lua_state, "fs" );

        const char* script = "print( 'Halo Welt!' )";
        luaL_loadstring( lua_state, script );
        lua_pcall( lua_state, 0, 0, 0 );
    }

    /*
    static int cd( lua_State* lua_state );
    static int pushd( lua_State* lua_state );
    static int popd( lua_State* lua_state );
    static int pwd( lua_State* lua_state );

    static int absolute( lua_State* lua_state );
    static int relative( lua_State* lua_state );
    static int root( lua_State* lua_state );
    static int initial( lua_State* lua_state );
    static int executable( lua_State* lua_state );
    static int home( lua_State* lua_state );

    static int native( lua_State* lua_state );
    static int branch( lua_State* lua_state );
    static int leaf( lua_State* lua_state );
    static int basename( lua_State* lua_state );
    static int extension( lua_State* lua_state );
    static int is_absolute( lua_State* lua_state );
    static int is_relative( lua_State* lua_state );

    static int exists( lua_State* lua_state );
    static int is_file( lua_State* lua_state );
    static int is_directory( lua_State* lua_state );
    static int ls( lua_State* lua_state );
    static int find( lua_State* lua_state );
    static int mkdir( lua_State* lua_state );
    static int rmdir( lua_State* lua_state );
    static int cp( lua_State* lua_state );
    static int rm( lua_State* lua_state );

    static int ls_iterator( lua_State* lua_state );
    static void push_directory_iterator( lua_State* lua_state, const boost::filesystem::directory_iterator& iterator );
    static boost::filesystem::directory_iterator* to_directory_iterator( lua_State* lua_state, int index );
    static int directory_iterator_gc( lua_State* lua_state );

    static int find_iterator( lua_State* lua_state );
    static void push_recursive_directory_iterator( lua_State* lua_state, const boost::filesystem::recursive_directory_iterator& iterator );
    static boost::filesystem::recursive_directory_iterator* to_recursive_directory_iterator( lua_State* lua_state, int index );
    static int recursive_directory_iterator_gc( lua_State* lua_state );
    */
}

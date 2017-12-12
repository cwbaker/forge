//
// LuaFs.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "LuaFs.hpp"
#include <sweet/fs/BasicPath.ipp>
#include <sweet/fs/DirectoryStack.hpp>
#include <sweet/fs/FileSystem.hpp>
#include <sweet/fs/fs.hpp>
#include <lua/lua.hpp>

using std::string;
using namespace sweet;
using namespace sweet::fs;

/**
// Constructor.
*/
LuaFs::LuaFs()
: lua_state_( NULL )
{
}

/**
// Destructor.
*/
LuaFs::~LuaFs()
{
    destroy();
}

/**
// Register the LuaFs API functions into the global 'fs' table.
//
// @param file_system
//  The FileSystem to use as underlying context for function calls like
//  'root()', 'initial()', etc and from which to retrieve the DirectoryStack
//  for calls to functions like 'cd()', 'pushd()', 'popd()', etc (assumed
//  not null).
//
// @param lua_state
//  The lua_State to create the LuaFs API within.
*/
void LuaFs::create( FileSystem* file_system, lua_State* lua_state )
{
    SWEET_ASSERT( file_system );
    SWEET_ASSERT( lua_state );

    destroy();
    lua_newtable( lua_state );
    create_with_existing_table( file_system, lua_state );
    lua_setglobal( lua_state, "fs" );
    lua_state_ = lua_state;
}

/**
// Add the LuaFs API functions into the table at the top of the stack.
//
// Registers the functions that make up the API into a table as closures
// with 'file_system' as their first upvalue.  The table is left on the 
// stack for manipulation by the caller (e.g. to call lua_global() or 
// otherwise store a reference for later use).
//
// @param file_system
//  The FileSystem to use as underlying context for function calls like
//  'root()', 'initial()', etc and from which to retrieve the DirectoryStack
//  for calls to functions like 'cd()', 'pushd()', 'popd()', etc (assumed
//  not null).
//
// @param lua_state
//  The lua_State to create the LuaFs API within (assumed not null).
*/
void LuaFs::create_with_existing_table( FileSystem* file_system, lua_State* lua_state )
{
    SWEET_ASSERT( file_system );
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, -1) );

    static const luaL_Reg functions[] = 
    {
        { "cd", &LuaFs::cd },
        { "pushd", &LuaFs::pushd },
        { "popd", &LuaFs::popd },
        { "pwd", &LuaFs::pwd },
        { "absolute", &LuaFs::absolute },
        { "relative", &LuaFs::relative },
        { "root", &LuaFs::root },
        { "initial", &LuaFs::initial },
        { "executable", &LuaFs::executable },
        { "home", &LuaFs::home },
        { NULL, NULL }
    };
    lua_pushlightuserdata( lua_state, file_system );
    luaL_setfuncs( lua_state, functions, 1 );
}

/**
// Set the value of the global 'fs' back to nil. 
*/
void LuaFs::destroy()
{
    if ( lua_state_ )
    {
        lua_pushglobaltable( lua_state_ );
        lua_pushstring( lua_state_, "fs" );
        lua_pushnil( lua_state_ );
        lua_rawset( lua_state_, -3 );
        lua_pop( lua_state_, 1 );
        lua_state_ = NULL;
    }
}

int LuaFs::cd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    directory_stack->change_directory( fs::Path(string(path, length)) );
    return 0;
}

int LuaFs::pushd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    directory_stack->push_directory( fs::Path(string(path, length)) );
    return 0;
}

int LuaFs::popd( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    directory_stack->pop_directory();
    return 0;
}

int LuaFs::pwd( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    const fs::Path& path = directory_stack->directory();
    lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    return 1;
}

int LuaFs::absolute( lua_State* lua_state )
{
    const int PATH = 1;
    const int BASE_PATH = 2;
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_checkstring( lua_state, BASE_PATH );
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), fs::Path(base_path) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( file_system );
        DirectoryStack* directory_stack = file_system->directory_stack();
        SWEET_ASSERT( directory_stack );
        const fs::Path& base_path = directory_stack->directory();
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaFs::relative( lua_State* lua_state )
{
    const int PATH = 1;
    const int BASE_PATH = 2;
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_checkstring( lua_state, BASE_PATH );
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), fs::Path(base_path) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( file_system );
        DirectoryStack* directory_stack = file_system->directory_stack();
        SWEET_ASSERT( directory_stack );
        const fs::Path& base_path = directory_stack->directory();
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaFs::root( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->root();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->root( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::initial( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->initial();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->initial( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::executable( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->executable();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->executable( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::home( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->home();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->home( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

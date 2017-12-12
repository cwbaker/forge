//
// LuaFSys.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "LuaFsys.hpp"
#include <sweet/fsys/BasicPath.ipp>
#include <sweet/fsys/DirectoryStack.hpp>
#include <sweet/fsys/FileSystem.hpp>
#include <sweet/fsys/fsys.hpp>
#include <lua/lua.hpp>

using std::string;
using namespace sweet;
using namespace sweet::fsys;

/**
// Constructor.
*/
LuaFsys::LuaFsys()
: lua_state_( NULL )
{
}

/**
// Destructor.
*/
LuaFsys::~LuaFsys()
{
    destroy();
}

/**
// Register the LuaFsys API functions into the global 'fs' table.
//
// @param file_system
//  The FileSystem to use as underlying context for function calls like
//  'root()', 'initial()', etc and from which to retrieve the DirectoryStack
//  for calls to functions like 'cd()', 'pushd()', 'popd()', etc (assumed
//  not null).
//
// @param lua_state
//  The lua_State to create the LuaFsys API within.
*/
void LuaFsys::create( FileSystem* file_system, lua_State* lua_state )
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
// Add the LuaFsys API functions into the table at the top of the stack.
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
//  The lua_State to create the LuaFsys API within (assumed not null).
*/
void LuaFsys::create_with_existing_table( FileSystem* file_system, lua_State* lua_state )
{
    SWEET_ASSERT( file_system );
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, -1) );

    static const luaL_Reg functions[] = 
    {
        { "cd", &LuaFsys::cd },
        { "pushd", &LuaFsys::pushd },
        { "popd", &LuaFsys::popd },
        { "pwd", &LuaFsys::pwd },
        { "absolute", &LuaFsys::absolute },
        { "relative", &LuaFsys::relative },
        { "root", &LuaFsys::root },
        { "initial", &LuaFsys::initial },
        { "executable", &LuaFsys::executable },
        { "home", &LuaFsys::home },
        { NULL, NULL }
    };
    lua_pushlightuserdata( lua_state, file_system );
    luaL_setfuncs( lua_state, functions, 1 );
}

/**
// Set the value of the global 'fs' back to nil. 
*/
void LuaFsys::destroy()
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

int LuaFsys::cd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    directory_stack->change_directory( fsys::Path(string(path, length)) );
    return 0;
}

int LuaFsys::pushd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    directory_stack->push_directory( fsys::Path(string(path, length)) );
    return 0;
}

int LuaFsys::popd( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    directory_stack->pop_directory();
    return 0;
}

int LuaFsys::pwd( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    DirectoryStack* directory_stack = file_system->directory_stack();
    SWEET_ASSERT( directory_stack );
    const fsys::Path& path = directory_stack->directory();
    lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    return 1;
}

int LuaFsys::absolute( lua_State* lua_state )
{
    const int PATH = 1;
    const int BASE_PATH = 2;
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_checkstring( lua_state, BASE_PATH );
        fsys::Path path = fsys::absolute( fsys::Path(luaL_checkstring(lua_state, PATH)), fsys::Path(base_path) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( file_system );
        DirectoryStack* directory_stack = file_system->directory_stack();
        SWEET_ASSERT( directory_stack );
        const fsys::Path& base_path = directory_stack->directory();
        fsys::Path path = fsys::absolute( fsys::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaFsys::relative( lua_State* lua_state )
{
    const int PATH = 1;
    const int BASE_PATH = 2;
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_checkstring( lua_state, BASE_PATH );
        fsys::Path path = fsys::absolute( fsys::Path(luaL_checkstring(lua_state, PATH)), fsys::Path(base_path) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( file_system );
        DirectoryStack* directory_stack = file_system->directory_stack();
        SWEET_ASSERT( directory_stack );
        const fsys::Path& base_path = directory_stack->directory();
        fsys::Path path = fsys::absolute( fsys::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaFsys::root( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fsys::Path& path = file_system->root();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fsys::Path path = file_system->root( fsys::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFsys::initial( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fsys::Path& path = file_system->initial();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fsys::Path path = file_system->initial( fsys::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFsys::executable( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fsys::Path& path = file_system->executable();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fsys::Path path = file_system->executable( fsys::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFsys::home( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fsys::Path& path = file_system->home();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fsys::Path path = file_system->home( fsys::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

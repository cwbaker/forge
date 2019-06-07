//
// LuaFileSystem.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaFileSystem.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

using std::string;
using boost::filesystem::directory_iterator;
using boost::filesystem::recursive_directory_iterator;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

static const char* DIRECTORY_ITERATOR_METATABLE = "boost::filesystem::directory_iterator";
static const char* RECURSIVE_DIRECTORY_ITERATOR_METATABLE = "boost::filesystem::recursive_directory_iterator";

LuaFileSystem::LuaFileSystem()
: lua_state_( NULL )
{
}

LuaFileSystem::~LuaFileSystem()
{
    destroy();
}

void LuaFileSystem::create( Forge* forge, lua_State* lua_state )
{
    SWEET_ASSERT( forge );
    SWEET_ASSERT( lua_state );

    destroy();

    static const luaL_Reg functions[] = 
    {
        { "exists", &LuaFileSystem::exists },
        { "is_file", &LuaFileSystem::is_file },
        { "is_directory", &LuaFileSystem::is_directory },
        { "ls", &LuaFileSystem::ls },
        { "find", &LuaFileSystem::find },
        { "mkdir", &LuaFileSystem::mkdir },
        { "rmdir", &LuaFileSystem::rmdir },
        { "cp", &LuaFileSystem::cp },
        { "rm", &LuaFileSystem::rm },
        { "touch", &LuaFileSystem::touch },
        { NULL, NULL }
    };
    lua_pushglobaltable( lua_state );
    lua_pushlightuserdata( lua_state, forge );
    luaL_setfuncs( lua_state, functions, 1 );
    lua_pop( lua_state, 1 );

    luaL_newmetatable( lua_state, DIRECTORY_ITERATOR_METATABLE );
    lua_pushstring( lua_state, "__gc" );
    lua_pushcfunction( lua_state, &LuaFileSystem::directory_iterator_gc );
    lua_rawset( lua_state, -3 );
    lua_pop( lua_state, 1 );

    luaL_newmetatable( lua_state, RECURSIVE_DIRECTORY_ITERATOR_METATABLE );
    lua_pushstring( lua_state, "__gc" );
    lua_pushcfunction( lua_state, &LuaFileSystem::recursive_directory_iterator_gc );
    lua_rawset( lua_state, -3 );
    lua_pop( lua_state, 1 );

    lua_state_ = lua_state;
}

void LuaFileSystem::destroy()
{
    if ( lua_state_ )
    {
        lua_pushstring( lua_state_, DIRECTORY_ITERATOR_METATABLE );
        lua_pushnil( lua_state_ );
        lua_rawset( lua_state_, LUA_REGISTRYINDEX );

        lua_pushstring( lua_state_, RECURSIVE_DIRECTORY_ITERATOR_METATABLE );
        lua_pushnil( lua_state_ );
        lua_rawset( lua_state_, LUA_REGISTRYINDEX );

        lua_state_ = NULL;
    }
}

int LuaFileSystem::exists( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    lua_pushboolean( lua_state, boost::filesystem::exists(path) ? 1 : 0 );
    return 1;
}

int LuaFileSystem::is_file( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    lua_pushboolean( lua_state, boost::filesystem::is_regular_file(path) ? 1 : 0 );
    return 1;
}

int LuaFileSystem::is_directory( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    lua_pushboolean( lua_state, boost::filesystem::is_directory(path) ? 1 : 0 );
    return 1;
}

int LuaFileSystem::ls( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    LuaFileSystem::push_directory_iterator( lua_state, directory_iterator(path) );
    lua_pushcclosure( lua_state, &LuaFileSystem::ls_iterator, 1 );
    return 1;
}

int LuaFileSystem::find( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    LuaFileSystem::push_recursive_directory_iterator( lua_state, recursive_directory_iterator(path) );
    lua_pushcclosure( lua_state, &LuaFileSystem::find_iterator, 1 );
    return 1;
}

int LuaFileSystem::mkdir( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    boost::filesystem::create_directories( path );
    return 0;
}

int LuaFileSystem::rmdir( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    boost::filesystem::remove_all( path );
    return 0;
}

int LuaFileSystem::cp( lua_State* lua_state )
{
    const int TO = 1;
    const int FROM = 2;
    boost::filesystem::path to = absolute( lua_state, TO );
    boost::filesystem::path from = absolute( lua_state, FROM );
    boost::filesystem::copy_file( from, to );
    return 0;
}

int LuaFileSystem::rm( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    boost::filesystem::remove( path );
    return 0;
}

int LuaFileSystem::touch( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::path path = absolute( lua_state, PATH );
    boost::filesystem::last_write_time( path, std::time(nullptr) );
    return 0;
}

int LuaFileSystem::ls_iterator( lua_State* lua_state )
{
    directory_iterator& iterator = *LuaFileSystem::to_directory_iterator( lua_state, lua_upvalueindex(1) );
    directory_iterator end;
    if ( iterator != end )
    {
        const boost::filesystem::directory_entry& entry = *iterator;
        lua_pushlstring( lua_state, entry.path().string().c_str(), entry.path().string().length() );
        ++iterator;
        return 1;
    }
    return 0;
}

void LuaFileSystem::push_directory_iterator( lua_State* lua_state, const boost::filesystem::directory_iterator& iterator )
{
    directory_iterator* other_iterator = (directory_iterator*) lua_newuserdata( lua_state, sizeof(directory_iterator) );
    new (other_iterator) directory_iterator( iterator );
    luaL_getmetatable( lua_state, DIRECTORY_ITERATOR_METATABLE );
    lua_setmetatable( lua_state, -2 );
}

boost::filesystem::directory_iterator* LuaFileSystem::to_directory_iterator( lua_State* lua_state, int index )
{
    directory_iterator* iterator = (directory_iterator*) luaL_checkudata( lua_state, index, DIRECTORY_ITERATOR_METATABLE );
    luaL_argcheck( lua_state, iterator != NULL, index, "directory iterator expected" );
    return iterator;
}

int LuaFileSystem::directory_iterator_gc( lua_State* lua_state )
{
    const int ITERATOR = 1;
    const directory_iterator* iterator = LuaFileSystem::to_directory_iterator( lua_state, ITERATOR );
    iterator->~directory_iterator();
    return 0;
}

int LuaFileSystem::find_iterator( lua_State* lua_state )
{
    recursive_directory_iterator& iterator = *LuaFileSystem::to_recursive_directory_iterator( lua_state, lua_upvalueindex(1) );
    recursive_directory_iterator end;
    if ( iterator != end )
    {
        const boost::filesystem::directory_entry& entry = *iterator;
        lua_pushlstring( lua_state, entry.path().string().c_str(), entry.path().string().length() );
        ++iterator;
        return 1;
    }
    return 0;
}

void LuaFileSystem::push_recursive_directory_iterator( lua_State* lua_state, const boost::filesystem::recursive_directory_iterator& iterator )
{
    recursive_directory_iterator* other_iterator = (recursive_directory_iterator*) lua_newuserdata( lua_state, sizeof(recursive_directory_iterator) );
    new (other_iterator) recursive_directory_iterator( iterator );
    luaL_getmetatable( lua_state, DIRECTORY_ITERATOR_METATABLE );
    lua_setmetatable( lua_state, -2 );
}

boost::filesystem::recursive_directory_iterator* LuaFileSystem::to_recursive_directory_iterator( lua_State* lua_state, int index )
{
    recursive_directory_iterator* iterator = (recursive_directory_iterator*) luaL_checkudata( lua_state, index, DIRECTORY_ITERATOR_METATABLE );
    luaL_argcheck( lua_state, iterator != NULL, index, "directory iterator expected" );
    return iterator;
}

int LuaFileSystem::recursive_directory_iterator_gc( lua_State* lua_state )
{
    const int ITERATOR = 1;
    const recursive_directory_iterator* iterator = LuaFileSystem::to_recursive_directory_iterator( lua_state, ITERATOR );
    iterator->~recursive_directory_iterator();
    return 0;
}

boost::filesystem::path LuaFileSystem::absolute( lua_State* lua_state, int index )
{
    const int FORGE = lua_upvalueindex( 1 );
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, index, &length );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    return forge->absolute( string(path, length) );
}

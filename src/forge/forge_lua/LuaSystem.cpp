//
// LuaSystem.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaSystem.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <forge/System.hpp>
#include <forge/Filter.hpp>
#include <forge/Arguments.hpp>
#include <forge/Scheduler.hpp>
#include <process/Environment.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

using std::string;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

LuaSystem::LuaSystem()
{
}

LuaSystem::~LuaSystem()
{
    destroy();
}

void LuaSystem::create( Forge* forge, lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    destroy();

    static const luaL_Reg functions[] = 
    {
        { "set_forge_hooks_library", &LuaSystem::set_forge_hooks_library },
        { "forge_hooks_library", &LuaSystem::forge_hooks_library },
        { "hash", &LuaSystem::hash },
        { "execute", &LuaSystem::execute },
        { "print", &LuaSystem::print },
        { "getenv", &LuaSystem::getenv },
        { "sleep", &LuaSystem::sleep },
        { "ticks", &LuaSystem::ticks },
        { "operating_system", &LuaSystem::operating_system },
        { NULL, NULL }
    };
    lua_pushglobaltable( lua_state );
    lua_pushlightuserdata( lua_state, forge );
    luaL_setfuncs( lua_state, functions, 1 );
    lua_pop( lua_state, 1 );
}

void LuaSystem::destroy()
{
}

int LuaSystem::set_forge_hooks_library( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int FORGE_HOOKS_LIBRARY = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    const char* forge_hooks_library = luaL_checkstring( lua_state, FORGE_HOOKS_LIBRARY );
    forge->set_forge_hooks_library( string(forge_hooks_library) );
    return 0;
}

int LuaSystem::forge_hooks_library( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    const string& forge_hooks_library = forge->forge_hooks_library();
    lua_pushlstring( lua_state, forge_hooks_library.c_str(), forge_hooks_library.size() );
    return 1;
}

int LuaSystem::hash( lua_State* lua_state )
{
    const int TABLE = 1;
    const int args = lua_gettop( lua_state );
    lua_Integer hash = 0;
    for ( int index = TABLE; index <= args; ++index )
    {
        luaL_checktype( lua_state, index, LUA_TTABLE );
        hash ^= hash_recursively( lua_state, index, false );
    }
    lua_pushinteger( lua_state, hash );
    return 1;
}

int LuaSystem::execute( lua_State* lua_state )
{
    try
    {
        const int FORGE = lua_upvalueindex( 1 );
        const int COMMAND = 1;
        const int COMMAND_LINE = 2;
        const int ENVIRONMENT = 3;
        const int DEPENDENCIES_FILTER = 4;
        const int STDOUT_FILTER = 5;
        const int STDERR_FILTER = 6;
        const int ARGUMENTS = 7;

        Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );

        size_t command_line_length = 0;
        const char* command_line = luaL_checklstring( lua_state, COMMAND_LINE, &command_line_length );

        unique_ptr<process::Environment> environment;
        if ( !lua_isnoneornil(lua_state, ENVIRONMENT) )
        {
            if ( !lua_istable(lua_state, ENVIRONMENT) )
            {
                lua_pushstring( lua_state, "Expected an environment table or nil as 3rd parameter" );
                return lua_error( lua_state );
            }
            
            environment.reset( new process::Environment );
            lua_pushnil( lua_state );
            while ( lua_next(lua_state, ENVIRONMENT) )
            {
                if ( lua_isstring(lua_state, -2) )
                {
                    const char* key = lua_tostring( lua_state, -2 );
                    const char* value = lua_tostring( lua_state, -1 );
                    environment->append( key, value );
                }
                lua_pop( lua_state, 1 );
            }
        }

        unique_ptr<Filter> dependencies_filter;
        if ( !lua_isnoneornil(lua_state, DEPENDENCIES_FILTER) )
        {
            if ( !lua_isfunction(lua_state, DEPENDENCIES_FILTER) && !lua_istable(lua_state, DEPENDENCIES_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            dependencies_filter.reset( new Filter(forge->lua_state(), lua_state, DEPENDENCIES_FILTER) );
        }

        unique_ptr<Filter> stdout_filter;
        if ( !lua_isnoneornil(lua_state, STDOUT_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDOUT_FILTER) && !lua_istable(lua_state, STDOUT_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 5th parameter (stdout filter)" );
                return lua_error( lua_state );
            }
            stdout_filter.reset( new Filter(forge->lua_state(), lua_state, STDOUT_FILTER) );
        }

        unique_ptr<Filter> stderr_filter;
        if ( !lua_isnoneornil(lua_state, STDERR_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDERR_FILTER) && !lua_istable(lua_state, STDERR_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 6th parameter (stderr filter)" );
                return lua_error( lua_state );
            }
            stderr_filter.reset( new Filter(forge->lua_state(), lua_state, STDERR_FILTER) );
        }

        unique_ptr<Arguments> arguments;
        if ( lua_gettop(lua_state) >= ARGUMENTS )
        {
            arguments.reset( new Arguments(forge->lua_state(), lua_state, ARGUMENTS, lua_gettop(lua_state) + 1) );
        }

        // Retrieve the command arguments last to avoid `luaL_tolstring()` 
        // pushing a string onto the stack that is then confused with the
        // variable length arguments gathered into the `Arguments` object.
        size_t command_length = 0;
        const char* command = luaL_tolstring( lua_state, COMMAND, &command_length );
        luaL_argcheck( lua_state, command_length > 0, COMMAND, "command must not be empty" );

        // Lift the command and command line strings out into explicit local
        // variables to workaround what seems to be a bug in Visual C++ 2017
        // that seems to elide copying those strings as well as destroying 
        // them when `lua_yield()` below throws as part of its yielding 
        // implementation.
        string command_string( command, command_length );
        string command_line_string( command_line, command_line_length );

        forge->scheduler()->execute(
            command_string,
            command_line_string,
            environment.release(),
            dependencies_filter.release(),
            stdout_filter.release(),
            stderr_filter.release(),
            arguments.release(),
            forge->context()
        );

        return lua_yield( lua_state, 0 );
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int LuaSystem::print( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TEXT = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    forge->output( luaL_checkstring(lua_state, TEXT) );
    return 0;
}

int LuaSystem::getenv( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int KEY = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    const char* key = luaL_checkstring( lua_state, KEY ); 
    const char* value = forge->system()->getenv( key );
    if ( value )
    {
        lua_pushstring( lua_state, value );
        return 1;
    }
    return 0;
}

int LuaSystem::sleep( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int MILLISECONDS = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    float milliseconds = static_cast<float>( luaL_checknumber(lua_state, MILLISECONDS) );
    forge->system()->sleep( milliseconds );
    return 0;
}

int LuaSystem::ticks( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    float ticks = forge->system()->ticks();
    lua_pushnumber( lua_state, ticks );
    return 1;
}

int LuaSystem::operating_system( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    const char* operating_system = forge->system()->operating_system();
    lua_pushstring( lua_state, operating_system );
    return 1;
}

lua_Integer LuaSystem::hash_recursively( lua_State* lua_state, int table, bool hash_integer_keys )
{
    const char HASH_KEYWORD [] = "__forge_hash";
    const size_t HASH_KEYWORD_LENGTH = sizeof(HASH_KEYWORD) - 1;

    // Hashed and sealed tables store their hash value in "__forge_hash".  
    // Checking for the presence of "__forge_hash" also prevents infinite
    // recursion in the case of cyclic references in the tables.
    lua_pushlstring( lua_state, HASH_KEYWORD, HASH_KEYWORD_LENGTH );
    if ( lua_rawget(lua_state, table) != LUA_TNIL )
    {
        lua_Integer hash = luaL_checkinteger( lua_state, -1 );
        lua_pop( lua_state, 1 );
        return hash;
    }
    lua_pop( lua_state, 1 );

    // Store a zero hash in the "__forge_hash" key in the table to prevent
    // infinite recursion in the case of cycles in the graph of table
    // relationships.
    lua_Integer hash = 0;
    lua_pushstring( lua_state, HASH_KEYWORD );
    lua_pushinteger( lua_state, hash );
    lua_rawset( lua_state, table );

    // Calculate the hash from each key and value stored in the table,
    // recursively for table values.
    lua_pushnil( lua_state );
    while ( lua_next(lua_state, table) )
    {
        uint64_t working_hash = fnv1a_start();

        int type = lua_type( lua_state, -2 );
        if ( type == LUA_TSTRING || (hash_integer_keys && type == LUA_TNUMBER) )
        {
            if ( type == LUA_TSTRING )
            {
                size_t length = 0;
                const char* key = lua_tolstring( lua_state, -2, &length );
                working_hash = fnv1a_append( working_hash, (const unsigned char*) key, length );
            }
            else
            {
                lua_Integer key = lua_tointeger( lua_state, -2 );
                working_hash = fnv1a_append( working_hash, (const unsigned char*) &key, sizeof(key) );
            }

            if ( lua_type(lua_state, -1) == LUA_TSTRING )
            {
                size_t length = 0;
                const char* value = lua_tolstring( lua_state, -1, &length );
                working_hash = fnv1a_append( working_hash, (const unsigned char*) value, length );
            }
            else if ( lua_isinteger(lua_state, -1) )
            {
                lua_Integer value = lua_tointeger( lua_state, -1 );
                working_hash = fnv1a_append( working_hash, (const unsigned char*) &value, sizeof(value) );
            }
            else if ( lua_isnumber(lua_state, -1) )
            {
                lua_Number value = lua_tonumber( lua_state, -1 );
                working_hash = fnv1a_append( working_hash, (const unsigned char*) &value, sizeof(value) );
            }
            else if ( lua_isboolean(lua_state, -1) )
            {
                bool value = lua_toboolean( lua_state, -1 ) == 1;
                working_hash = fnv1a_append( working_hash, (const unsigned char*) &value, sizeof(value) );
            }
            else if ( lua_istable(lua_state, -1) )
            {
                hash ^= hash_recursively( lua_state, lua_gettop(lua_state), true );
            }

            hash ^= working_hash;
        }
        lua_pop( lua_state, 1 );
    }

    // Recursively calculate hashes from fields in tables that this table
    // inherits from.
    int type = luaL_getmetafield( lua_state, table, "__index" );
    if ( type != LUA_TNIL )
    {
        if ( type == LUA_TTABLE )
        {
            hash ^= hash_recursively( lua_state, lua_gettop(lua_state), false );
        }
        lua_pop( lua_state, 1 );
    }

    // Store the hash in the "__forge_hash" key in the table to make it
    // available afterwards and to mark the table as hashed and sealed.
    lua_pushstring( lua_state, HASH_KEYWORD );
    lua_pushinteger( lua_state, hash );
    lua_rawset( lua_state, table );

    return hash;
}

uint64_t LuaSystem::fnv1a_start()
{
    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;
    return FNV_OFFSET_BASIS;
}

uint64_t LuaSystem::fnv1a_append( uint64_t hash, const unsigned char* data, size_t length )
{
    SWEET_ASSERT( data );
    SWEET_ASSERT( length >= 0 );
    for ( size_t i = 0; i < length; ++i )
    {
        const uint64_t FNV_PRIME = 0x100000001b3;
        hash = (hash ^ data[i]) * FNV_PRIME;
    }
    return hash;
}

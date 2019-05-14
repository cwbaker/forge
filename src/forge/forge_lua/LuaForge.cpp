//
// LuaForge.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaForge.hpp"
#include "LuaFileSystem.hpp"
#include "LuaSystem.hpp"
#include "LuaContext.hpp"
#include "LuaGraph.hpp"
#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <forge/Filter.hpp>
#include <forge/Arguments.hpp>
#include <forge/Scheduler.hpp>
#include <process/Process.hpp>
#include <process/Environment.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <meow_hash/meow_intrinsics.h>
#if defined BUILD_OS_MACOS
// Ignore unused function warning for 'MeowHash_Accelerated'
#pragma clang diagnostic ignored "-Wunused-function"
#endif
#include <meow_hash/meow_hash.h>
#include <meow_hash/more/meow_more.h>
#include <string>

using std::string;
using std::unique_ptr;
using boost::filesystem::path;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::luaxx;
using namespace sweet::forge;

LuaForge::LuaForge( Forge* forge )
: forge_( nullptr ),
  lua_state_( nullptr ),
  lua_file_system_( nullptr ),
  lua_context_( nullptr ),
  lua_graph_( nullptr ),
  lua_system_( nullptr ),
  lua_target_( nullptr ),
  lua_target_prototype_( nullptr )
{
    create( forge );
}

LuaForge::~LuaForge()
{
    destroy();
}

lua_State* LuaForge::lua_state() const
{
    SWEET_ASSERT( lua_state_ );
    return lua_state_;
}

LuaTarget* LuaForge::lua_target() const
{
    SWEET_ASSERT( lua_target_ );
    return lua_target_;
}

LuaTargetPrototype* LuaForge::lua_target_prototype() const
{
    SWEET_ASSERT( lua_target_prototype_ );
    return lua_target_prototype_;
}

void LuaForge::create( Forge* forge )
{
    SWEET_ASSERT( forge );

    destroy();

    forge_ = forge;
    lua_state_ = luaxx_newstate();
    lua_file_system_ = new LuaFileSystem;
    lua_context_ = new LuaContext;
    lua_graph_ = new LuaGraph;
    lua_system_ = new LuaSystem;
    lua_target_ = new LuaTarget;
    lua_target_prototype_ = new LuaTargetPrototype;

    static const luaL_Reg functions[] = 
    {
        { "set_maximum_parallel_jobs", &LuaForge::set_maximum_parallel_jobs },
        { "maximum_parallel_jobs", &LuaForge::maximum_parallel_jobs },
        { "set_stack_trace_enabled", &LuaForge::set_stack_trace_enabled },
        { "stack_trace_enabled", &LuaForge::stack_trace_enabled },
        { "set_forge_hooks_library", &LuaForge::set_forge_hooks_library },
        { "forge_hooks_library", &LuaForge::forge_hooks_library },
        { "hash", &LuaForge::hash },
        { "execute", &LuaForge::execute },
        { "print", &LuaForge::print },
        { nullptr, nullptr }
    };

    luaxx_create( lua_state_, forge, FORGE_TYPE );
    luaxx_push( lua_state_, forge );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_context_->create( forge, lua_state_ );
    lua_file_system_->create( forge, lua_state_ );
    lua_graph_->create( forge, lua_state_ );
    lua_system_->create( forge, lua_state_ );
    lua_target_->create( lua_state_ );
    lua_target_prototype_->create( lua_state_, lua_target_ );
    lua_setglobal( lua_state_, "forge" );

    // Set `package.path` to load forge scripts stored in `../lua` relative 
    // to the `forge` executable.  The value of `package.path` may be 
    // overridden again in `forge.lua` before requiring modules.
    path first_path = forge_->executable( "../lua/?.lua" );
    path second_path = forge_->executable( "../lua/?/init.lua" );
    string path = first_path.generic_string() + ";" + second_path.generic_string();
    lua_getglobal( lua_state_, "package" );
    lua_pushlstring( lua_state_, path.c_str(), path.size() );
    lua_setfield( lua_state_, -2, "path" );
    lua_pop( lua_state_, 1 );
}

void LuaForge::destroy()
{
    delete lua_target_prototype_;
    lua_target_prototype_ = nullptr;

    delete lua_target_;
    lua_target_ = nullptr;

    delete lua_system_;
    lua_system_ = nullptr;

    delete lua_graph_;
    lua_graph_ = nullptr;

    delete lua_context_;
    lua_context_ = nullptr;

    delete lua_file_system_;
    lua_file_system_ = nullptr;

    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, forge_ );
        lua_close( lua_state_ );
    }

    lua_state_ = nullptr;
    forge_ = nullptr;
}

/**
// Extract assignments from \e assignments_and_commands and use them to 
// assign values to global variables.
//
// This is used to accept variable assignments on the command line and have 
// them available for scripts to use for configuration when commands are
// executed.
//
// @param assignments
//  The assignments specified on the command line used to create global 
//  variables before any scripts are loaded (e.g. 'variant=release' etc).
*/
void LuaForge::assign_global_variables( const std::vector<std::string>& assignments )
{
    for ( std::vector<std::string>::const_iterator i = assignments.begin(); i != assignments.end(); ++i )
    {
        std::string::size_type position = i->find( "=" );
        if ( position != std::string::npos )
        {
            std::string attribute = i->substr( 0, position );
            std::string value = i->substr( position + 1, std::string::npos );
            lua_pushlstring( lua_state_, value.c_str(), value.size() );
            lua_setglobal( lua_state_, attribute.c_str() );
        }
    }
}

int LuaForge::set_maximum_parallel_jobs( lua_State* lua_state )
{
    const int FORGE = 1;
    const int MAXIMUM_PARALLEL_JOBS = 2;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    int maximum_parallel_jobs = static_cast<int>( luaL_checkinteger(lua_state, MAXIMUM_PARALLEL_JOBS) );
    forge->set_maximum_parallel_jobs( maximum_parallel_jobs );
    return 0;
}

int LuaForge::maximum_parallel_jobs( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    int maximum_parallel_jobs = forge->maximum_parallel_jobs();
    lua_pushinteger( lua_state, maximum_parallel_jobs );
    return 1;
}

int LuaForge::set_stack_trace_enabled( lua_State* lua_state )
{
    const int FORGE = 1;
    const int STACK_TRACE_ENABLED = 2;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    bool stack_trace_enabled = lua_toboolean( lua_state, STACK_TRACE_ENABLED ) != 0;
    forge->set_stack_trace_enabled( stack_trace_enabled );
    return 0;
}

int LuaForge::stack_trace_enabled( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    bool stack_trace_enabled = forge->stack_trace_enabled();
    lua_pushboolean( lua_state, stack_trace_enabled ? 1 : 0 );
    return 1;
}

int LuaForge::set_forge_hooks_library( lua_State* lua_state )
{
    const int FORGE = 1;
    const int FORGE_HOOKS_LIBRARY = 2;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    const char* forge_hooks_library = luaL_checkstring( lua_state, FORGE_HOOKS_LIBRARY );
    forge->set_forge_hooks_library( string(forge_hooks_library) );
    return 0;
}

int LuaForge::forge_hooks_library( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    const string& forge_hooks_library = forge->forge_hooks_library();
    lua_pushlstring( lua_state, forge_hooks_library.c_str(), forge_hooks_library.size() );
    return 1;
}

int LuaForge::hash( lua_State* lua_state )
{
    const int TABLE = 2;
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

int LuaForge::execute( lua_State* lua_state )
{
    try
    {
        const int FORGE = 1;
        const int COMMAND = 2;
        const int COMMAND_LINE = 3;
        const int ENVIRONMENT = 4;
        const int DEPENDENCIES_FILTER = 5;
        const int STDOUT_FILTER = 6;
        const int STDERR_FILTER = 7;
        const int ARGUMENTS = 8;

        Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );

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

int LuaForge::print( lua_State* lua_state )
{
    const int FORGE = 1;
    const int TEXT = 2;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    forge->output( luaL_checkstring(lua_state, TEXT) );
    return 0;
}

lua_Integer LuaForge::hash_recursively( lua_State* lua_state, int table, bool hash_integer_keys )
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

    // Initiate iteration with the nil left on top of the stack by the call
    // to `lua_getfield()` that failed to find a "__forge_hash" field.
    lua_Integer hash = 0;
    while ( lua_next(lua_state, table) )
    {
        meow_hash_state state;
        MeowHashBegin( &state );

        bool hash_value = false;
        int type = lua_type( lua_state, -2 );
        if ( type == LUA_TSTRING || (hash_integer_keys && type == LUA_TNUMBER) )
        {
            if ( type == LUA_TSTRING )
            {
                size_t length = 0;
                const char* key = lua_tolstring( lua_state, -2, &length );
                MeowHashAbsorb( &state, length, (void*) key );
                hash_value = true;
            }
            else
            {
                lua_Integer key = lua_tointeger( lua_state, -2 );
                MeowHashAbsorb( &state, sizeof(key), (void*) &key );
                hash_value = true;
            }

            if ( lua_type(lua_state, -1) == LUA_TSTRING )
            {
                size_t length = 0;
                const char* value = lua_tolstring( lua_state, -1, &length );
                MeowHashAbsorb( &state, length, (void*) value );
            }
            else if ( lua_isinteger(lua_state, -1) )
            {
                lua_Integer value = lua_tointeger( lua_state, -1 );
                MeowHashAbsorb( &state, sizeof(value), (void*) &value );
            }
            else if ( lua_isnumber(lua_state, -1) )
            {
                lua_Number value = lua_tonumber( lua_state, -1 );
                MeowHashAbsorb( &state, sizeof(value), (void*) &value );
            }
            else if ( lua_isboolean(lua_state, -1) )
            {
                bool value = lua_toboolean( lua_state, -1 ) == 1;
                MeowHashAbsorb( &state, sizeof(value), (void*) &value );
            }
            else if ( lua_istable(lua_state, -1) )
            {
                hash ^= hash_recursively( lua_state, lua_gettop(lua_state), true );
            }

            meow_hash working_hash = MeowHashEnd( &state, 0xc0dedbad );
            hash ^= MeowU64From( working_hash, 0 );
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

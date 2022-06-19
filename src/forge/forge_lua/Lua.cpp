//
// Lua.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Lua.hpp"
#include "LuaFileSystem.hpp"
#include "LuaSystem.hpp"
#include "LuaContext.hpp"
#include "LuaGraph.hpp"
#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include "LuaToolset.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <string>

using std::string;
using boost::filesystem::path;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

Lua::Lua( Forge* forge )
: forge_( nullptr )
, lua_state_( nullptr )
, lua_file_system_( nullptr )
, lua_context_( nullptr )
, lua_graph_( nullptr )
, lua_system_( nullptr )
, lua_target_( nullptr )
, lua_target_prototype_( nullptr )
, lua_toolset_( nullptr )
{
    create( forge );
}

Lua::~Lua()
{
    destroy();
}

lua_State* Lua::lua_state() const
{
    SWEET_ASSERT( lua_state_ );
    return lua_state_;
}

LuaTarget* Lua::lua_target() const
{
    SWEET_ASSERT( lua_target_ );
    return lua_target_;
}

LuaTargetPrototype* Lua::lua_target_prototype() const
{
    SWEET_ASSERT( lua_target_prototype_ );
    return lua_target_prototype_;
}

LuaToolset* Lua::lua_toolset() const
{
    SWEET_ASSERT( lua_toolset_ );
    return lua_toolset_;
}

void Lua::create( Forge* forge )
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
    lua_toolset_ = new LuaToolset;

    luaxx_create( lua_state_, forge, FORGE_TYPE );
    luaxx_push( lua_state_, forge );
    lua_target_->create( lua_state_, forge );
    lua_target_prototype_->create( lua_state_, forge, lua_target_ );
    lua_toolset_->create( lua_state_ );
    lua_setglobal( lua_state_, "forge" );

    lua_context_->create( forge, lua_state_ );
    lua_file_system_->create( forge, lua_state_ );
    lua_graph_->create( forge, lua_state_ );
    lua_system_->create( forge, lua_state_ );

    // Set `package.path` to load forge scripts stored in `../lua` relative 
    // to the `forge` executable.  The value of `package.path` may be 
    // overridden again in `forge.lua` before requiring modules.
    path first_path = forge_->executable( "../lua/?.lua" );
    path second_path = forge_->executable( "../lua/?/init.lua" );
    string path = first_path.generic_string() + ";" + second_path.generic_string();
    set_package_path( path );
}

void Lua::destroy()
{
    delete lua_toolset_;
    lua_toolset_ = nullptr;

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
// Assign variable values from *assignments* to global variables in Lua.
//
// This is used to convert assignments from the command line into Lua globals
// for scripts to use when commands are executed.  Assignments without any
// value (e.g. 'variant=') set the variable to nil.
//
// @param assignments
//  The assignments specified on the command line used to create global 
//  variables before any scripts are loaded (e.g. 'variant=release' etc).
*/
void Lua::assign_variables( const std::vector<std::string>& assignments )
{
    for ( std::vector<std::string>::const_iterator i = assignments.begin(); i != assignments.end(); ++i )
    {
        std::string::size_type position = i->find( "=" );
        if ( position != std::string::npos )
        {
            std::string attribute = i->substr( 0, position );
            if ( position + 1 < i->size() )
            {
                std::string value = i->substr( position + 1, std::string::npos );
                lua_pushlstring( lua_state_, value.c_str(), value.size() );
            }
            else
            {
                lua_pushnil( lua_state_ );
            }
            lua_setglobal( lua_state_, attribute.c_str() );
        }
    }
}

/**
// Set the Lua module search path in `package.path`.
//
// @param path
//  The value to set `package.path` to.
*/
void Lua::set_package_path( const std::string& path )
{
    SWEET_ASSERT( lua_state_ );
    lua_getglobal( lua_state_, "package" );
    lua_pushlstring( lua_state_, path.c_str(), path.size() );
    lua_setfield( lua_state_, -2, "path" );
    lua_pop( lua_state_, 1 );
}

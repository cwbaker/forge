//
// LuaToolset.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaToolset.hpp"
#include "types.hpp"
#include <forge/Toolset.hpp>
#include <forge/ToolsetPrototype.hpp>
#include <forge/Forge.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

using std::string;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

const char* LuaToolset::TOOLSET_METATABLE = "forge.Toolset";

LuaToolset::LuaToolset()
: lua_state_( nullptr )
{
}

LuaToolset::~LuaToolset()
{
    destroy();
}

void LuaToolset::create( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    destroy();

    lua_state_ = lua_state;
    luaxx_create( lua_state_, this, TOOLSET_TYPE );

    static const luaL_Reg functions[] = 
    {
        { "id", &LuaToolset::id },
        { "prototype", &LuaToolset::prototype },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_pop( lua_state_, 1 );

    // Set the metatable for `Toolset` to redirect calls to create new
    // toolsets in `Toolset.create()`.
    luaxx_push( lua_state_, this );
    lua_newtable( lua_state_ );
    lua_pushcfunction( lua_state_, &LuaToolset::create_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_setmetatable( lua_state_, -2 );
    lua_pop( lua_state_, 1 );

    // Create a metatable for toolsets that redirects index operations to
    // `Toolset` and calls to `Toolset.inherit()`.
    luaL_newmetatable( lua_state_, TOOLSET_METATABLE );
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaToolset::id );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pop( lua_state_, 1 );

    // Set `forge.Toolset` to this object.
    const int FORGE = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, FORGE, "Toolset" );
}

void LuaToolset::destroy()
{
    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, this );
        lua_state_ = nullptr;
    }
}

void LuaToolset::create_toolset( Toolset* toolset )
{
    SWEET_ASSERT( toolset );
    luaxx_create( lua_state_, toolset, TOOLSET_TYPE );
    update_toolset( toolset );
}

void LuaToolset::update_toolset( Toolset* toolset )
{
    ToolsetPrototype* toolset_prototype = toolset->prototype();
    SWEET_ASSERT( toolset_prototype );
    if ( toolset_prototype )
    {
        luaxx_push( lua_state_, toolset );
        luaxx_push( lua_state_, toolset_prototype );
        lua_setmetatable( lua_state_, -2 );
        lua_pop( lua_state_, 1 );
    }
    else
    {
        luaxx_push( lua_state_, toolset );
        luaL_setmetatable( lua_state_, TOOLSET_METATABLE );
        lua_pop( lua_state_, 1 );
    }
}

void LuaToolset::destroy_toolset( Toolset* toolset )
{
    SWEET_ASSERT( toolset );
    luaxx_destroy( lua_state_, toolset );
}

int LuaToolset::id( lua_State* lua_state )
{
    const int TOOLSET = 1;
    Toolset* toolset = (Toolset*) luaxx_to( lua_state, TOOLSET, TOOLSET_TYPE );
    luaL_argcheck( lua_state, toolset != nullptr, TOOLSET, "nil toolset" );
    if ( toolset )
    {
        const string& id = toolset->id();
        lua_pushlstring( lua_state, id.c_str(), id.length() );
        return 1;
    }
    return 0;
}

int LuaToolset::prototype( lua_State* lua_state )
{
    const int TOOLSET = 1;
    Toolset* toolset = (Toolset*) luaxx_to( lua_state, TOOLSET, TOOLSET_TYPE );
    luaL_argcheck( lua_state, toolset != nullptr, TOOLSET, "nil toolset" );
    if ( toolset )
    {
        ToolsetPrototype* toolset_prototype = toolset->prototype();
        if ( toolset_prototype )
        {
            luaxx_push( lua_state, toolset_prototype );
            return 1;
        }
    }
    return 0;
}

/**
// Redirect calls made on `forge.Toolset()` to `Toolset.new()`.
//
// Removes the `Toolset` table, passed as part of the metamethod call, from
// the stack and calls through to `Toolset.new()` passing through 
// the identifier and any other arguments.
//
// ~~~lua
// function target_call_metamethod( _, forge, identifier, ... )
//     return Toolset.new( identifier, ... );
// end
// ~~~
*/
int LuaToolset::create_call_metamethod( lua_State* lua_state )
{
    const int TOOLSET = 1;
    const int VARARGS = 2;
    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, TOOLSET, "new" );
    lua_pushvalue( lua_state, TOOLSET );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_callk( lua_state, args, 1, 0, &LuaToolset::continue_create_call_metamethod );
    return continue_create_call_metamethod( lua_state, LUA_OK, 0 );
}

/**
// Continue create call metamethods that yield.
//
// @return
//  Returns 1.
*/
int LuaToolset::continue_create_call_metamethod( lua_State* /*lua_state*/, int /*status*/, lua_KContext /*context*/ )
{
    return 1;
}

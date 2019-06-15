//
// LuaToolset.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaToolset.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

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
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_pop( lua_state_, 1 );

    // Set the metatable for `Toolset` to redirect calls to create new forges 
    // in `Toolset.create()` via `LuaToolset::create_call_metamethod()`.
    luaxx_push( lua_state_, this );
    lua_newtable( lua_state_ );
    lua_pushcfunction( lua_state_, &LuaToolset::create_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_setmetatable( lua_state_, -2 );
    lua_pop( lua_state_, 1 );

    // Create a metatable for forges that redirects index operations to 
    // `Toolset` and calls to `Toolset.inherit()` via 
    // `LuaToolset::inherit_call_metamethod()`.
    luaL_newmetatable( lua_state_, TOOLSET_METATABLE );
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaToolset::inherit_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
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

int LuaToolset::create_call_metamethod( lua_State* lua_state )
{
    const int TOOLSET = 1;
    const int VARARGS = 2;
    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, TOOLSET, "create" );
    lua_pushvalue( lua_state, TOOLSET );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_call( lua_state, args, 1 );
    return 1;
}

/**
// Redirect calls made on forge objects to `Toolset.inherit()`.
//
// The call to `Toolset.inherit()` may be overridden by providing `inherit()`
// methods on `Toolset` and/or individual forge objects.
//
// ~~~lua
// function inherit_call_metamethod( forge, ... )
//     local inherit_function = forge.inherit;
//     inherit_function( forge, ... );
//     return forge;
// end
// ~~~
*/
int LuaToolset::inherit_call_metamethod( lua_State* lua_state )
{
    const int TOOLSET = 1;
    const int VARARGS = 2;
    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, TOOLSET, "inherit" );
    lua_pushvalue( lua_state, TOOLSET );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_call( lua_state, args + 1, 1 );
    return 1;
}

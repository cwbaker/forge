//
// LuaTargetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <forge/Graph.hpp>
#include <forge/TargetPrototype.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <string>

using std::string;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

static const char* TARGET_PROTOTYPE_METATABLE = "forge.TargetPrototype";

LuaTargetPrototype::LuaTargetPrototype()
: lua_state_( nullptr )
{
}

LuaTargetPrototype::~LuaTargetPrototype()
{
    destroy();
}

void LuaTargetPrototype::create( lua_State* lua_state, Forge* forge, LuaTarget* lua_target )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( forge );
    SWEET_ASSERT( lua_target );

    destroy();
    lua_state_ = lua_state;

    luaxx_create( lua_state_, this, TARGET_PROTOTYPE_TYPE );

    // Set the metatable for `TargetPrototype` to redirect calls to create new
    // target prototypes.
    luaxx_push( lua_state_, this );
    lua_newtable( lua_state_ );
    lua_pushlightuserdata( lua_state, forge );
    lua_pushcclosure( lua_state_, &LuaTargetPrototype::create_target_prototype_call_metamethod, 1 );
    lua_setfield( lua_state_, -2, "__call" );
    lua_setmetatable( lua_state_, -2 );
    lua_pop( lua_state_, 1 );

    // Create a metatable for target prototypes to redirect index operations
    // to `forge.Target` and calls to `TargetPrototype.create()` via
    // `LuaTargetPrototype::create_call_metamethod()`.
    luaL_newmetatable( lua_state_, TARGET_PROTOTYPE_METATABLE );
    luaxx_push( lua_state_, lua_target );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaTargetPrototype::create_target_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_pop( lua_state_, 1 );

    // Set `forge.TargetPrototype` to this object.
    const int FORGE = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, FORGE, "TargetPrototype" );

    // Set global `TargetPrototype` to this object.
    luaxx_push( lua_state_, this );
    lua_setglobal( lua_state_, "TargetPrototype" );
}

void LuaTargetPrototype::destroy()
{
    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, this );
        lua_state_ = nullptr;
    }
}

void LuaTargetPrototype::create_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( target_prototype );

    luaxx_create( lua_state_, target_prototype, TARGET_PROTOTYPE_TYPE );
    luaxx_push( lua_state_, target_prototype );
    luaL_setmetatable( lua_state_, TARGET_PROTOTYPE_METATABLE );
    lua_pushvalue( lua_state_, -1 );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaTarget::filename );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pushcfunction( lua_state_, &LuaTarget::depend_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_pushstring( lua_state_, LuaTarget::TARGET_METATABLE );
    lua_setfield( lua_state_, -2, "__name" );
    lua_pop( lua_state_, 1 );
}

void LuaTargetPrototype::destroy_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( target_prototype );
    luaxx_destroy( lua_state_, target_prototype );
}

/**
// Redirect calls made on `TargetPrototype` to create new target prototypes.
*/
int LuaTargetPrototype::create_target_prototype_call_metamethod( lua_State* lua_state )
{
    try
    {
        const int FORGE = lua_upvalueindex( 1 );
        const int TARGET_PROTOTYPE = 1;
        const int IDENTIFIER = 2;

        // Ignore `TargetPrototype` passed as first parameter.
        (void) TARGET_PROTOTYPE;

        string id = luaL_checkstring( lua_state, IDENTIFIER );
        Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
        TargetPrototype* target_prototype = forge->graph()->add_target_prototype( id );
        forge->create_target_prototype_lua_binding( target_prototype );
        luaxx_push( lua_state, target_prototype );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

/**
// Redirect calls made on target prototype objects to create functions.
//
// ~~~lua
// function create_call_metamethod( target_prototype, forge, identifier, ... )
//     local create = target_prototype.create;
//     return create( forge, identifier, target_prototype, ... );
// end
// ~~~
*/
int LuaTargetPrototype::create_target_call_metamethod( lua_State* lua_state )
{
    const int TARGET_PROTOTYPE = 1;
    const int TOOLSET = 2;
    const int IDENTIFIER = 3;
    const int VARARGS = 4;

    if ( lua_type(lua_state, IDENTIFIER) == LUA_TNONE )
    {
        return luaL_argerror( lua_state, IDENTIFIER - 1, "string expected" );
    }

    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, TARGET_PROTOTYPE, "create" );
    lua_pushvalue( lua_state, TOOLSET );
    lua_pushvalue( lua_state, IDENTIFIER );
    lua_pushvalue( lua_state, TARGET_PROTOTYPE );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_call( lua_state, args, 1 );
    return 1;
}

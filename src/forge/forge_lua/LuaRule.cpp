//
// LuaRule.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaRule.hpp"
#include "LuaTarget.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <forge/Graph.hpp>
#include <forge/Rule.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <string>

using std::string;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

static const char* RULE_METATABLE = "forge.Rule";

LuaRule::LuaRule()
: lua_state_( nullptr )
{
}

LuaRule::~LuaRule()
{
    destroy();
}

void LuaRule::create( lua_State* lua_state, Forge* forge, LuaTarget* lua_target )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( forge );
    SWEET_ASSERT( lua_target );

    destroy();
    lua_state_ = lua_state;

    luaxx_create( lua_state_, this, RULE_TYPE );

    // Set the metatable for `Rule` to redirect calls to create new
    // target prototypes.
    luaxx_push( lua_state_, this );
    lua_newtable( lua_state_ );
    lua_pushlightuserdata( lua_state, forge );
    lua_pushcclosure( lua_state_, &LuaRule::create_rule_call_metamethod, 1 );
    lua_setfield( lua_state_, -2, "__call" );
    lua_setmetatable( lua_state_, -2 );
    lua_pop( lua_state_, 1 );

    // Create a metatable for target prototypes to redirect index operations
    // to `forge.Target` and calls to `Rule.create()` via
    // `LuaRule::create_call_metamethod()`.
    luaL_newmetatable( lua_state_, RULE_METATABLE );
    luaxx_push( lua_state_, lua_target );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaRule::id );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pushcfunction( lua_state_, &LuaRule::create_target_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_pop( lua_state_, 1 );

    // Set `forge.Rule` to this object.
    const int FORGE = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, FORGE, "Rule" );

    // Set global `Rule` to this object.
    luaxx_push( lua_state_, this );
    lua_setglobal( lua_state_, "Rule" );
}

void LuaRule::destroy()
{
    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, this );
        lua_state_ = nullptr;
    }
}

void LuaRule::create_rule( Rule* rule )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( rule );

    luaxx_create( lua_state_, rule, RULE_TYPE );
    luaxx_push( lua_state_, rule );
    luaL_setmetatable( lua_state_, RULE_METATABLE );
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

void LuaRule::destroy_rule( Rule* rule )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( rule );
    luaxx_destroy( lua_state_, rule );
}

int LuaRule::id( lua_State* lua_state )
{
    const int RULE = 1;
    Rule* rule = (Rule*) luaxx_to( lua_state, RULE, RULE_TYPE );
    luaL_argcheck( lua_state, rule != nullptr, RULE, "nil target prototype" );
    if ( rule )
    {
        const string& id = rule->id();
        lua_pushlstring( lua_state, id.c_str(), id.length() );
        return 1;
    }
    return 0;
}

/**
// Redirect calls made on `Rule` to create new target prototypes.
*/
int LuaRule::create_rule_call_metamethod( lua_State* lua_state )
{
    try
    {
        const int FORGE = lua_upvalueindex( 1 );
        const int RULE = 1;
        const int IDENTIFIER = 2;

        // Ignore `Rule` passed as first parameter.
        (void) RULE;

        string id = luaL_checkstring( lua_state, IDENTIFIER );
        Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
        Rule* rule = forge->graph()->add_rule( id );
        forge->create_rule_lua_binding( rule );
        luaxx_push( lua_state, rule );
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
// function create_call_metamethod( rule, forge, identifier, ... )
//     local create = rule.create;
//     return create( forge, identifier, rule, ... );
// end
// ~~~
*/
int LuaRule::create_target_call_metamethod( lua_State* lua_state )
{
    const int RULE = 1;
    const int TOOLSET = 2;
    const int IDENTIFIER = 3;
    const int VARARGS = 4;

    if ( lua_type(lua_state, IDENTIFIER) == LUA_TNONE )
    {
        return luaL_argerror( lua_state, IDENTIFIER - 1, "string expected" );
    }

    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, RULE, "create" );
    lua_pushvalue( lua_state, TOOLSET );
    lua_pushvalue( lua_state, IDENTIFIER );
    lua_pushvalue( lua_state, RULE );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_call( lua_state, args, 1 );
    return 1;
}

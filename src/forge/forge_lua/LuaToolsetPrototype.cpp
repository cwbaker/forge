//
// LuaToolsetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaToolsetPrototype.hpp"
#include "LuaToolset.hpp"
#include "types.hpp"
#include <forge/ToolsetPrototype.hpp>
#include <forge/Graph.hpp>
#include <forge/Forge.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

using std::string;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

static const char* TOOLSET_PROTOTYPE_METATABLE = "forge.ToolsetPrototype";

LuaToolsetPrototype::LuaToolsetPrototype()
: lua_state_( nullptr )
{
}

LuaToolsetPrototype::~LuaToolsetPrototype()
{
    destroy();
}

void LuaToolsetPrototype::create( lua_State* lua_state, Forge* forge, LuaToolset* lua_toolset )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( forge );
    SWEET_ASSERT( lua_toolset );

    destroy();
    lua_state_ = lua_state;
    luaxx_create( lua_state_, this, TOOLSET_PROTOTYPE_TYPE );

    // Set the metatable for `ToolsetPrototype` to redirect calls to create
    // new toolset prototypes via `LuaToolsetPrototype::toolset_prototype_call_metamethod()`.
    luaxx_push( lua_state_, this );
    lua_newtable( lua_state_ );
    lua_pushlightuserdata( lua_state, forge );
    lua_pushcclosure( lua_state_, &LuaToolsetPrototype::create_toolset_prototype_call_metamethod, 1 );
    lua_setfield( lua_state_, -2, "__call" );
    lua_setmetatable( lua_state_, -2 );
    lua_pop( lua_state_, 1 );

    // Create a metatable for toolset prototypes to redirect calls to create
    // new toolsets, index operations to `Toolset`, and string conversion to
    // return the toolset prototype's identifier.
    luaL_newmetatable( lua_state_, TOOLSET_PROTOTYPE_METATABLE );
    luaxx_push( lua_state_, lua_toolset );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaToolsetPrototype::create_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_pushcfunction( lua_state_, &LuaToolsetPrototype::id );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pop( lua_state_, 1 );

    // Set `forge.ToolsetPrototype` to this object.
    const int FORGE = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, FORGE, "ToolsetPrototype" );

    // Set global `ToolsetPrototype` to this object.
    luaxx_push( lua_state_, this );
    lua_setglobal( lua_state_, "ToolsetPrototype" );
}

void LuaToolsetPrototype::destroy()
{
    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, this );
        lua_state_ = nullptr;
    }
}

void LuaToolsetPrototype::create_toolset_prototype( ToolsetPrototype* toolset_prototype )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( toolset_prototype );

    luaxx_create( lua_state_, toolset_prototype, TOOLSET_PROTOTYPE_TYPE );
    luaxx_push( lua_state_, toolset_prototype );
    luaL_setmetatable( lua_state_, TOOLSET_PROTOTYPE_METATABLE );
    lua_pushvalue( lua_state_, -1 );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaToolset::id );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pushstring( lua_state_, LuaToolset::TOOLSET_METATABLE );
    lua_setfield( lua_state_, -2, "__name" );
    lua_pop( lua_state_, 1 );
}

void LuaToolsetPrototype::destroy_toolset_prototype( ToolsetPrototype* toolset_prototype )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( toolset_prototype );
    luaxx_destroy( lua_state_, toolset_prototype );
}

/**
// Return the identifier of this toolset prototype.
//
// @return
//  The identifier of this toolset prototype.
*/
int LuaToolsetPrototype::id( lua_State* lua_state )
{
    const int TOOLSET_PROTOTYPE = 1;
    ToolsetPrototype* toolset_prototype = (ToolsetPrototype*) luaxx_to( lua_state, TOOLSET_PROTOTYPE, TOOLSET_PROTOTYPE_TYPE );
    luaL_argcheck( lua_state, toolset_prototype != nullptr, TOOLSET_PROTOTYPE, "nil toolset_prototype prototype" );
    if ( toolset_prototype )
    {
        const string& id = toolset_prototype->id();
        lua_pushlstring( lua_state, id.c_str(), id.length() );
        return 1;
    }
    return 0;
}

/**
// Redirect calls made on `ToolsetPrototype` to create new toolset prototypes.
*/
int LuaToolsetPrototype::create_toolset_prototype_call_metamethod( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TOOLSET_PROTOTYPE = 1;
    const int IDENTIFIER = 2;

    try
    {
        (void) TOOLSET_PROTOTYPE;
        Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
        string id = luaL_checkstring( lua_state, IDENTIFIER );        
        ToolsetPrototype* toolset_prototype = forge->graph()->add_toolset_prototype( id );
        forge->create_toolset_prototype_lua_binding( toolset_prototype );
        luaxx_push( lua_state, toolset_prototype );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

/**
// Redirect calls made on toolset prototype objects to create functions.
//
// ~~~lua
// function create_call_metamethod( toolset_prototype, ... )
//     local create_function = toolset_prototype.create;
//     return create_function( toolset_prototype, ... );
// end
// ~~~
*/
int LuaToolsetPrototype::create_call_metamethod( lua_State* lua_state )
{
    const int TOOLSET_PROTOTYPE = 1;
    const int VARARGS = 2;
    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, TOOLSET_PROTOTYPE, "new" );
    lua_pushvalue( lua_state, TOOLSET_PROTOTYPE );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_callk( lua_state, args, 1, 0, &LuaToolsetPrototype::continue_create_call_metamethod );
    return continue_create_call_metamethod( lua_state, LUA_OK, 0 );
}

int LuaToolsetPrototype::continue_create_call_metamethod( lua_State* /*lua_state*/, int /*status*/, lua_KContext /*context*/ )
{
    return 1;
}

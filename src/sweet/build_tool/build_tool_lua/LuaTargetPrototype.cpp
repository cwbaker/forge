//
// LuaTargetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include "LuaBuildTool.hpp"
#include "types.hpp"
#include <sweet/build_tool/TargetPrototype.hpp>
#include <sweet/luaxx/luaxx.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::build_tool;

static const char* TARGET_PROTOTYPE_METATABLE = "build.TargetPrototype";

LuaTargetPrototype::LuaTargetPrototype()
: lua_state_( nullptr )
{
}

LuaTargetPrototype::~LuaTargetPrototype()
{
    destroy();
}

void LuaTargetPrototype::create( lua_State* lua_state, LuaTarget* lua_target )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_target );

    destroy();

    lua_state_ = lua_state;
    luaxx_create( lua_state_, this, TARGET_PROTOTYPE_TYPE );

    luaL_newmetatable( lua_state_, TARGET_PROTOTYPE_METATABLE );
    lua_push_object( lua_state_, lua_target->target_prototype() );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pop( lua_state_, 1 );

    const int BUILD = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, BUILD, "TargetPrototype" );
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
    lua_pop( lua_state_, 1 );
}

void LuaTargetPrototype::destroy_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( target_prototype );
    luaxx_destroy( lua_state_, target_prototype );
}

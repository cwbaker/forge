//
// LuaTargetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include "LuaBuildTool.hpp"
#include <sweet/build_tool/TargetPrototype.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/lua/LuaObject.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

static const char* TARGET_PROTOTYPE_METATABLE = "build.TargetPrototype";

LuaTargetPrototype::LuaTargetPrototype()
: lua_( NULL ),
  target_prototype_prototype_( NULL )
{
}

LuaTargetPrototype::~LuaTargetPrototype()
{
    destroy();
}

void LuaTargetPrototype::create( lua::Lua* lua, LuaTarget* lua_target )
{
    SWEET_ASSERT( lua );
    SWEET_ASSERT( lua_target );

    destroy();

    lua_ = lua;
    target_prototype_prototype_ = new lua::LuaObject( *lua_ );

    lua_State* lua_state = lua_->get_lua_state();
    luaL_newmetatable( lua_state, TARGET_PROTOTYPE_METATABLE );
    lua_push_object( lua_state, lua_target->target_prototype() );
    lua_setfield( lua_state, -2, "__index" );
    lua_pop( lua_state, 1 );

    const int BUILD = 1;
    lua_push_object( lua_state, target_prototype_prototype_ );
    lua_setfield( lua_state, BUILD, "TargetPrototype" );
}

void LuaTargetPrototype::destroy()
{
    delete target_prototype_prototype_;
    target_prototype_prototype_ = NULL;

    lua_ = NULL;
}

void LuaTargetPrototype::create_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_ );
    SWEET_ASSERT( target_prototype );

    lua_State* lua_state = lua_->get_lua_state();
    lua_push_object( lua_state, target_prototype );
    luaL_setmetatable( lua_state, TARGET_PROTOTYPE_METATABLE );
    lua_push_value<rtti::Type>( lua_state, SWEET_STATIC_TYPEID(TargetPrototype) );
    lua_setfield( lua_state, -2, lua::TYPE_KEYWORD );
    lua_pushlightuserdata( lua_state, target_prototype );
    lua_setfield( lua_state, -2, lua::THIS_KEYWORD );
    lua_push_object( lua_state, target_prototype );
    lua_setfield( lua_state, -2, "__index" );
    lua_pushcfunction( lua_state, &LuaTarget::filename );
    lua_setfield( lua_state, -2, "__tostring" );
    lua_pop( lua_state, 1 );
}

void LuaTargetPrototype::destroy_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_ );
    SWEET_ASSERT( target_prototype );
    lua_State* lua_state = lua_->get_lua_state();
    lua_destroy_object( lua_state, target_prototype );
}

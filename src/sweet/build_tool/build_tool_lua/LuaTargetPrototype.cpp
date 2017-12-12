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

LuaTargetPrototype::LuaTargetPrototype()
: lua_( NULL ),
  lua_target_( NULL ),
  target_prototype_metatable_( NULL ),
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
    lua_target_ = lua_target;
    target_prototype_metatable_ = new lua::LuaObject( *lua_ );
    target_prototype_prototype_ = new lua::LuaObject( *lua_ );

    target_prototype_metatable_->members()
        ( "__index", target_prototype_prototype_ )
    ;

    target_prototype_prototype_->members()
        .type( SWEET_STATIC_TYPEID(TargetPrototype) )
        ( "id", &TargetPrototype::id )
    ;
}

void LuaTargetPrototype::destroy()
{
    delete target_prototype_prototype_;
    target_prototype_prototype_ = NULL;

    delete target_prototype_metatable_;
    target_prototype_metatable_ = NULL;

    lua_ = NULL;
    lua_target_ = NULL;
}

void LuaTargetPrototype::create_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_ );
    SWEET_ASSERT( lua_target_ );
    SWEET_ASSERT( target_prototype );

    lua_->members( target_prototype )
        .type( SWEET_STATIC_TYPEID(TargetPrototype) )
        .metatable( *target_prototype_metatable_ )
        .this_pointer( target_prototype )
        ( "__index", target_prototype )
        ( "__tostring", raw(LuaTarget::filename) )
    ;
    AddMember add_member = lua_->members( target_prototype );
    lua_target_->register_functions( add_member );
}

void LuaTargetPrototype::destroy_target_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_ );
    SWEET_ASSERT( target_prototype );
    lua_->destroy( target_prototype );
}

//
// Filter.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Filter.hpp"
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using namespace sweet::build_tool;

Filter::Filter()
: lua_state_( nullptr ),
  reference_( LUA_NOREF )
{
}

Filter::Filter( lua_State* lua_state, int position )
: lua_state_( lua_state ),
  reference_( LUA_NOREF )
{
    SWEET_ASSERT( lua_state_ );
    lua_pushvalue( lua_state_, position );
    reference_ = luaL_ref( lua_state_, LUA_REGISTRYINDEX );
}

Filter::Filter( const Filter& value )
: lua_state_( value.lua_state_ ),
  reference_( LUA_NOREF )
{
    if ( lua_state_ )
    {
        lua_rawgeti( lua_state_, LUA_REGISTRYINDEX, value.reference_ );
        reference_ = luaL_ref( lua_state_, LUA_REGISTRYINDEX );
    }
}

Filter& Filter::operator=( const Filter& value )
{
    if ( this != &value )
    {
        if ( lua_state_ && reference_ != LUA_NOREF )
        {
            luaL_unref( lua_state_, LUA_REGISTRYINDEX, reference_ );
            reference_ = LUA_NOREF;
            lua_state_ = nullptr;
        }
        
        lua_state_ = value.lua_state_;
        
        if ( lua_state_ )
        {
            lua_rawgeti( lua_state_, LUA_REGISTRYINDEX, value.reference_ );
            reference_ = luaL_ref( lua_state_, LUA_REGISTRYINDEX );
        }
    }
    return *this;
}

Filter::~Filter()
{
    if ( lua_state_ )
    {
        luaL_unref( lua_state_, LUA_REGISTRYINDEX, reference_ );
        reference_ = LUA_NOREF;
        lua_state_ = nullptr;
    }
}

int Filter::reference() const
{
    return reference_;
}

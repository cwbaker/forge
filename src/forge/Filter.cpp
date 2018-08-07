//
// Filter.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Filter.hpp"
#include <assert/assert.hpp>
#include <lua.hpp>

using namespace sweet::forge;

Filter::Filter()
: lua_state_( nullptr ),
  reference_( LUA_NOREF )
{
}

Filter::Filter( lua_State* lua_state, lua_State* calling_lua_state, int position )
: lua_state_( lua_state ),
  reference_( LUA_NOREF )
{
    SWEET_ASSERT( lua_state_ );
    lua_pushvalue( calling_lua_state, position );
    reference_ = luaL_ref( calling_lua_state, LUA_REGISTRYINDEX );
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
        lua_State* lua_state = value.lua_state_;
        int reference = LUA_NOREF;
        if ( lua_state )
        {
            lua_rawgeti( lua_state, LUA_REGISTRYINDEX, value.reference_ );
            reference = luaL_ref( lua_state, LUA_REGISTRYINDEX );
        }

        if ( lua_state_ && reference_ != LUA_NOREF )
        {
            luaL_unref( lua_state_, LUA_REGISTRYINDEX, reference_ );
            reference_ = LUA_NOREF;
            lua_state_ = nullptr;
        }
        
        lua_state_ = lua_state;
        reference_ = reference;
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

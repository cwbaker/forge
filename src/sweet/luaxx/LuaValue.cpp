//
// LuaValue.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaValue.hpp"
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using namespace sweet::luaxx;

LuaValue::LuaValue()
: lua_state_( NULL )
{
}

LuaValue::LuaValue( lua_State* lua_state, int position )
: lua_state_( lua_state )
{
    SWEET_ASSERT( lua_state_ );
    lua_pushvalue( lua_state_, position );
    lua_pushlightuserdata( lua_state_, this );
    lua_insert( lua_state_, -2 );
    lua_rawset( lua_state_, LUA_REGISTRYINDEX );    
}

LuaValue::LuaValue( lua_State* lua_state, lua_State* coroutine, int position )
: lua_state_( lua_state )
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( coroutine );
    lua_pushvalue( coroutine, position );
    lua_pushlightuserdata( coroutine, this );
    lua_insert( coroutine, -2 );
    lua_rawset( coroutine, LUA_REGISTRYINDEX );    
}

LuaValue::LuaValue( const LuaValue& value )
: lua_state_( value.lua_state_ )
{
    if ( lua_state_ )
    {        
        lua_pushlightuserdata( lua_state_, this );
        lua_pushlightuserdata( lua_state_, static_cast<void*>(const_cast<LuaValue*>(&value)) );
        lua_rawget( lua_state_, LUA_REGISTRYINDEX );
        lua_rawset( lua_state_, LUA_REGISTRYINDEX );        
    }
}

LuaValue& LuaValue::operator=( const LuaValue& value )
{
    if ( this != &value )
    {
        if ( lua_state_ && lua_state_ != value.lua_state_ )
        {
            lua_pushlightuserdata( lua_state_, this );
            lua_pushnil( lua_state_ );
            lua_rawset( lua_state_, LUA_REGISTRYINDEX );
        }
        
        lua_state_ = value.lua_state_;
        
        if ( lua_state_ )
        {
            lua_pushlightuserdata( lua_state_, this );
            lua_pushlightuserdata( lua_state_, static_cast<void*>(const_cast<LuaValue*>(&value)) );
            lua_rawget( lua_state_, LUA_REGISTRYINDEX );
            lua_rawset( lua_state_, LUA_REGISTRYINDEX );        
        }
    }
    return *this;
}

LuaValue::~LuaValue()
{
    if ( lua_state_ )
    {
        lua_pushlightuserdata( lua_state_, this );
        lua_pushnil( lua_state_ );
        lua_rawset( lua_state_, LUA_REGISTRYINDEX );
        lua_state_ = nullptr;
    }
}

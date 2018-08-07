//
// Arguments.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Arguments.hpp"
#include <assert/assert.hpp>
#include <lua.hpp>

using std::vector;
using namespace sweet;
using namespace sweet::forge;

Arguments::Arguments( lua_State* lua_state, lua_State* calling_lua_state, int begin, int end )
: lua_state_( lua_state ),
  arguments_()
{
    SWEET_ASSERT( lua_state_ );
    SWEET_ASSERT( begin >= 0 && end >= 0 && end >= begin );

    if ( end - begin > 0 )
    {
        arguments_.reserve( end - begin );
        for ( int i = begin; i < end; ++i )
        {
            lua_pushvalue( calling_lua_state, i );
            arguments_.push_back( luaL_ref(calling_lua_state, LUA_REGISTRYINDEX) );
        }
    }
}

Arguments::~Arguments()
{
    while ( lua_state_ && !arguments_.empty() )
    {
        luaL_unref( lua_state_, LUA_REGISTRYINDEX, arguments_.back() );
        arguments_.pop_back();
    }
}

int Arguments::push_arguments( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    for ( auto i = arguments_.begin(); i != arguments_.end(); ++i )
    {
        lua_rawgeti( lua_state, LUA_REGISTRYINDEX, *i );
    }
    return int(arguments_.size());
}

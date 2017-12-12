//
// Arguments.cpp
// Copyright (c) 2011 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Arguments.hpp"
#include <sweet/lua/AddParameter.hpp>
#include <sweet/lua/LuaValue.hpp>
#include <sweet/assert/assert.hpp>

using std::vector;
using namespace sweet;
using namespace sweet::build_tool;

Arguments::Arguments( lua::Lua& lua, lua_State* lua_state, int begin, int end )
: arguments_()
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( begin >= 0 && end >= 0 && end >= begin );

    if ( end - begin > 0 )
    {
        arguments_.reserve( end - begin );
        for ( int i = begin; i < end; ++i )
        {
            arguments_.push_back( lua::LuaValue(lua, lua_state, i) );
        }
    }
}

void Arguments::push_arguments( lua::AddParameter& add_parameter )
{
    vector<lua::LuaValue>::const_iterator argument = arguments_.begin(); 
    vector<lua::LuaValue>::const_iterator arguments_end = arguments_.end(); 
    while ( argument != arguments_end )
    {
        add_parameter( *argument );
        ++argument;
    }
}

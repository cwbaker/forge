//
// Pattern.cpp
// Copyright (c) 2008 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Pattern.hpp"

using namespace sweet;
using namespace sweet::build_tool;

Pattern::Pattern( const std::string& regex, lua::Lua& lua, lua_State* lua_state, int position )
: regex_( regex ),
  function_( lua, lua_state, position )
{
}

const std::regex& Pattern::get_regex() const
{
    return regex_;
}

const lua::LuaValue& Pattern::get_function() const
{
    return function_;
}

#ifndef SWEET_BUILD_TOOL_PATTERN_HPP_INCLUDED
#define SWEET_BUILD_TOOL_PATTERN_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/lua/LuaValue.hpp>
#include <regex>
#include <string>

namespace sweet
{

namespace build_tool
{

/**
// A regular expression to match and a Lua function to call when that pattern
// is matched.
*/
class SWEET_BUILD_TOOL_DECLSPEC Pattern
{
    std::regex regex_; ///< The regular expression to match for this Pattern.
    lua::LuaValue function_; ///< The function to call when the expression for this Pattern matches.

    public:
        Pattern( const std::string& regex, lua::Lua& lua, lua_State* lua_state, int position );
        const std::regex& regex() const;
        const lua::LuaValue& function() const;
};

}

}

#endif

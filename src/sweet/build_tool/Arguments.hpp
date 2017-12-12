#ifndef SWEET_BUILD_TOOL_ARGUMENTS_HPP_INCLUDED
#define SWEET_BUILD_TOOL_ARGUMENTS_HPP_INCLUDED

#include <sweet/lua/LuaValue.hpp>
#include <vector>

namespace sweet
{

namespace lua
{

class AddParameter;
class Lua;

}

namespace build_tool
{

/**
// Hold references to the extra arguments passed in calls to "`execute()`" and
// "`scan()`" so that they can be passed to match calls.
*/
class Arguments
{
    std::vector<lua::LuaValue> arguments_; ///< The variable arguments to pass to match functions.

    public:
        Arguments( lua::Lua& lua, lua_State* lua_state, int begin, int end );
        void push_arguments( lua::AddParameter& add_parameter );
};

}

}

#endif
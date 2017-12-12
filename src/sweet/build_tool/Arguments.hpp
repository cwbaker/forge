#ifndef SWEET_BUILD_TOOL_ARGUMENTS_HPP_INCLUDED
#define SWEET_BUILD_TOOL_ARGUMENTS_HPP_INCLUDED

#include <vector>

struct lua_State;

namespace sweet
{

namespace lua
{

class AddParameter;

}

namespace build_tool
{

/**
// Hold references to the extra arguments passed in calls to "`execute()`" and
// "`scan()`" so that they can be passed to match calls.
*/
class Arguments
{
    lua_State* lua_state_;
    std::vector<int> arguments_; ///< The variable arguments passed through `execute()` and `postorder()` 

    public:
        Arguments( lua_State* lua_state, int begin, int end );
        ~Arguments();
        void push_arguments( lua::AddParameter& add_parameter );
};

}

}

#endif

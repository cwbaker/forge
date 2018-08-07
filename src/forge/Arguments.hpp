#ifndef FORGE_ARGUMENTS_HPP_INCLUDED
#define FORGE_ARGUMENTS_HPP_INCLUDED

#include <vector>

struct lua_State;

namespace sweet
{

namespace lua
{

class AddParameter;

}

namespace forge
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
        Arguments( lua_State* lua_state, lua_State* calling_lua_state, int begin, int end );
        ~Arguments();
        int push_arguments( lua_State* lua_state );
};

}

}

#endif

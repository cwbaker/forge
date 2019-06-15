#ifndef FORGE_LUATOOLSET_HPP_INCLUDED
#define FORGE_LUATOOLSET_HPP_INCLUDED

#include <ctime>

struct lua_State;

namespace sweet
{

namespace forge
{

class Forge;

class LuaToolset
{
    lua_State* lua_state_; ///< The main Lua virtual machine to create the target API in.

public:
    static const char* TOOLSET_METATABLE;

    LuaToolset();
    ~LuaToolset();

    void create( lua_State* lua_state );
    void destroy();

    static int create_call_metamethod( lua_State* lua_state );
    static int inherit_call_metamethod( lua_State* lua_state );
};

}

}

#endif
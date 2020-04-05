#ifndef FORGE_LUATOOLSETPROTOTYPE_HPP_INCLUDED
#define FORGE_LUATOOLSETPROTOTYPE_HPP_INCLUDED

#include <lua.hpp>

namespace sweet
{

namespace forge
{

class LuaToolset;
class ToolsetPrototype;

class LuaToolsetPrototype
{
    lua_State* lua_state_; ///< The main Lua virtual machine to create the toolset API in.

public:
    LuaToolsetPrototype();
    ~LuaToolsetPrototype();
    void create( lua_State* lua_state, LuaToolset* lua_toolset );
    void destroy();
    void create_toolset_prototype( ToolsetPrototype* toolset_prototype );
    void destroy_toolset_prototype( ToolsetPrototype* toolset_prototype );
    static int id( lua_State* lua_state );
    static int toolset_prototype_call_metamethod( lua_State* lua_state );
    static int create_call_metamethod( lua_State* lua_state );
    static int continue_create_call_metamethod( lua_State* lua_state, int /*status*/, lua_KContext /*context*/ );
};
    
}

}

#endif

#ifndef FORGE_LUATARGETPROTOTYPE_HPP_INCLUDED
#define FORGE_LUATARGETPROTOTYPE_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace forge
{

class LuaTarget;
class TargetPrototype;
class Forge;

class LuaTargetPrototype
{
    lua_State* lua_state_; ///< The main Lua virtual machine to create the target API in.

public:
    LuaTargetPrototype();
    ~LuaTargetPrototype();
    void create( lua_State* lua_state, Forge* forge, LuaTarget* lua_target );
    void destroy();
    void create_target_prototype( TargetPrototype* target_prototype );
    void destroy_target_prototype( TargetPrototype* target_prototype );
    static int create_target_prototype_call_metamethod( lua_State* lua_state );
    static int create_target_call_metamethod( lua_State *lua_state );
};
    
}

}

#endif

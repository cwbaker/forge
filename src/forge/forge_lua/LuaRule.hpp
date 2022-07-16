#ifndef FORGE_LUATARGETPROTOTYPE_HPP_INCLUDED
#define FORGE_LUATARGETPROTOTYPE_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace forge
{

class LuaTarget;
class Rule;
class Forge;

class LuaRule
{
    lua_State* lua_state_; ///< The main Lua virtual machine to create the target API in.

public:
    LuaRule();
    ~LuaRule();
    void create( lua_State* lua_state, Forge* forge, LuaTarget* lua_target );
    void destroy();
    void create_rule( Rule* rule );
    void destroy_rule( Rule* rule );
    static int id( lua_State* lua_state );
    static int create_rule_call_metamethod( lua_State* lua_state );
    static int create_target_call_metamethod( lua_State *lua_state );
};
    
}

}

#endif

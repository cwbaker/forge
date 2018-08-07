#ifndef FORGE_LUASYSTEM_HPP_INCLUDED
#define FORGE_LUASYSTEM_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace forge
{

class Forge;

class LuaSystem
{
public:
    LuaSystem();
    ~LuaSystem();
    void create( Forge* forge, lua_State* lua_state );
    void destroy();

private:
    static int getenv( lua_State* lua_state );
    static int sleep( lua_State* lua_state );
    static int ticks( lua_State* lua_state );
    static int operating_system( lua_State* lua_state );
};

}

}

#endif

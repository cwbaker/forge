#ifndef FORGE_LUASYSTEM_HPP_INCLUDED
#define FORGE_LUASYSTEM_HPP_INCLUDED

#include <lua.hpp>

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
    static int set_forge_hooks_library( lua_State* lua_state );
    static int forge_hooks_library( lua_State* lua_state );
    static int hash( lua_State* lua_state );
    static int execute( lua_State* lua_state );
    static int print( lua_State* lua_state );
    static int getenv( lua_State* lua_state );
    static int sleep( lua_State* lua_state );
    static int ticks( lua_State* lua_state );
    static int operating_system( lua_State* lua_state );
    static lua_Integer hash_recursively( lua_State* lua_state, int table, bool hash_integer_keys );
};

}

}

#endif

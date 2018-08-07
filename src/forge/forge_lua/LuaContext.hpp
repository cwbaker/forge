#ifndef FORGE_LUACONTEXT_HPP_INCLUDED
#define FORGE_LUACONTEXT_HPP_INCLUDED

struct lua_State;

namespace sweet
{
    
namespace forge
{

class Forge;

class LuaContext
{
public:
    LuaContext();
    ~LuaContext();
    void create( Forge* forge, lua_State* lua_state );
    void destroy();
    
private:
    static int cd( lua_State* lua_state );
    static int pushd( lua_State* lua_state );
    static int popd( lua_State* lua_state );
    static int pwd( lua_State* lua_state );

    static int absolute( lua_State* lua_state );
    static int relative( lua_State* lua_state );
    static int root( lua_State* lua_state );
    static int initial( lua_State* lua_state );
    static int executable( lua_State* lua_state );
    static int home( lua_State* lua_state );

    static int lower( lua_State* lua_state );
    static int upper( lua_State* lua_state );
    static int native( lua_State* lua_state );
    static int branch( lua_State* lua_state );
    static int leaf( lua_State* lua_state );
    static int basename( lua_State* lua_state );
    static int extension( lua_State* lua_state );
    static int is_absolute( lua_State* lua_state );
    static int is_relative( lua_State* lua_state );
};
    
}

}

#endif

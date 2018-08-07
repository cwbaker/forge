#ifndef SWEET_BUILD_TOOL_LUASYSTEM_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUASYSTEM_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace build_tool
{

class BuildTool;

class LuaSystem
{
public:
    LuaSystem();
    ~LuaSystem();
    void create( BuildTool* build_tool, lua_State* lua_state );
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

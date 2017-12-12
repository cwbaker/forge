#ifndef SWEET_BUILD_TOOL_LUAGRAPH_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUAGRAPH_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace build_tool
{

class Target;
class BuildTool;

class LuaGraph
{
public:
    LuaGraph();
    ~LuaGraph();
    void create( BuildTool* build_tool, lua_State* lua_state );
    void destroy();

private:
    static Target* add_target( lua_State* lua_state );
    static int target_prototype( lua_State* lua_state );
    static int file( lua_State* lua_state );
    static int target( lua_State* lua_state );
    static int find_target( lua_State* lua_state );
    static int anonymous( lua_State* lua_state );
    static int working_directory( lua_State* lua_state );
    static int buildfile( lua_State* lua_state );
    static int postorder( lua_State* lua_state );
    static int print_dependencies( lua_State* lua_state );
    static int print_namespace( lua_State* lua_state );
    static int wait( lua_State* lua_state );
    static int clear( lua_State* lua_state );
    static int load_binary( lua_State* lua_state );
    static int save_binary( lua_State* lua_state );
};

}

}

#endif

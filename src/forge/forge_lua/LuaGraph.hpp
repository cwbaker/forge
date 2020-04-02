#ifndef FORGE_LUAGRAPH_HPP_INCLUDED
#define FORGE_LUAGRAPH_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace forge
{

class Target;
class Forge;

class LuaGraph
{
public:
    LuaGraph();
    ~LuaGraph();
    void create( Forge* forge, lua_State* lua_state );
    void destroy();

private:
    static int add_toolset_prototype( lua_State* lua_state );
    static int add_toolset( lua_State* lua_state );
    static int all_toolsets_iterator( lua_State* lua_state );
    static int all_toolsets( lua_State* lua_state );
    static int add_target_prototype( lua_State* lua_state );
    static int add_target( lua_State* lua_state );
    static int find_target( lua_State* lua_state );
    static int anonymous( lua_State* lua_state );
    static int current_buildfile( lua_State* lua_state );
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

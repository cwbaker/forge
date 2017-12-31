#ifndef SWEET_BUILD_TOOL_LUABUILDTOOL_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUABUILDTOOL_HPP_INCLUDED

#include <vector>

struct lua_State;

namespace sweet
{
    
namespace build_tool
{

class Target;
class TargetPrototype;
class BuildTool;
class LuaFileSystem;
class LuaContext;
class LuaGraph;
class LuaSystem;
class LuaTarget;
class LuaTargetPrototype;

class LuaBuildTool
{
    BuildTool* build_tool_;
    lua_State* lua_state_;
    LuaFileSystem* lua_file_system_;
    LuaContext* lua_context_;
    LuaGraph* lua_graph_;
    LuaSystem* lua_system_;
    LuaTarget* lua_target_;
    LuaTargetPrototype* lua_target_prototype_;

public:
    LuaBuildTool( BuildTool* build_tool );
    ~LuaBuildTool();
    lua_State* lua_state() const;
    LuaTarget* lua_target() const;
    LuaTargetPrototype* lua_target_prototype() const;
    void create( BuildTool* build_tool );
    void destroy();
    void assign_global_variables( const std::vector<std::string>& assignments );

private:
    static int set_maximum_parallel_jobs( lua_State* lua_state );
    static int maximum_parallel_jobs( lua_State* lua_state );
    static int set_stack_trace_enabled( lua_State* lua_state );
    static int stack_trace_enabled( lua_State* lua_state );
    static int set_build_hooks_library( lua_State* lua_state );
    static int build_hooks_library( lua_State* lua_state );
    static int default_package_path( lua_State* lua_state );
    static int execute( lua_State* lua_state );
    static int print( lua_State* lua_state );
};
    
}

}

#endif

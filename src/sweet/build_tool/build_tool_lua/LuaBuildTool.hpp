#ifndef SWEET_BUILD_TOOL_LUABUILDTOOL_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUABUILDTOOL_HPP_INCLUDED

#include <sweet/lua/Lua.hpp>

struct lua_State;

namespace sweet
{
    
namespace lua
{

class Lua;

}

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
    lua::Lua* lua_;
    LuaFileSystem* lua_file_system_;
    LuaContext* lua_context_;
    LuaGraph* lua_graph_;
    LuaSystem* lua_system_;
    LuaTarget* lua_target_;
    LuaTargetPrototype* lua_target_prototype_;

public:
    LuaBuildTool( BuildTool* build_tool, lua::Lua* lua );
    ~LuaBuildTool();
    LuaTarget* lua_target() const;
    LuaTargetPrototype* lua_target_prototype() const;
    void create( BuildTool* build_tool, lua::Lua* lua );
    void destroy();

private:
    static int set_maximum_parallel_jobs( lua_State* lua_state );
    static int maximum_parallel_jobs( lua_State* lua_state );
    static int set_stack_trace_enabled( lua_State* lua_state );
    static int stack_trace_enabled( lua_State* lua_state );
    static int set_build_hooks_library( lua_State* lua_state );
    static int build_hooks_library( lua_State* lua_state );
    static int execute( lua_State* lua_state );
    static int print( lua_State* lua_state );
};
    
}

}

SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::TargetPrototype, LuaByReference );
SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::Target, LuaByReference );
SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::BuildTool, LuaByReference );

#endif

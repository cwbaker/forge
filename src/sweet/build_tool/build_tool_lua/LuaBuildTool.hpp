#ifndef SWEET_BUILD_TOOL_LUABUILDTOOL_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUABUILDTOOL_HPP_INCLUDED

#include <sweet/lua/Lua.hpp>
#include <sweet/lua/LuaObject.hpp>

struct lua_State;

namespace sweet
{
    
namespace lua
{

class LuaObject;
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
    lua::LuaObject* build_;
    LuaFileSystem* lua_file_system_;
    LuaContext* lua_context_;
    LuaGraph* lua_graph_;
    LuaSystem* lua_system_;
    LuaTarget* lua_target_;
    LuaTargetPrototype* lua_target_prototype_;

public:
    LuaBuildTool( BuildTool* build_tool, lua::Lua* lua );
    ~LuaBuildTool();

    lua::LuaObject* build() const;
    LuaTarget* lua_target() const;
    LuaTargetPrototype* lua_target_prototype() const;

    void create( BuildTool* build_tool, lua::Lua* lua );
    void destroy();

private:
    static int execute( lua_State* lua_state );
    static int print( lua_State* lua_state );
};
    
}

}

SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::TargetPrototype, LuaByReference );
SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::Target, LuaByReference );

#endif

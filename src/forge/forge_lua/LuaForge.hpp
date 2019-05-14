#ifndef FORGE_LUAFORGE_HPP_INCLUDED
#define FORGE_LUAFORGE_HPP_INCLUDED

#include <vector>
#include <string>
#include <lua.hpp>

struct lua_State;

namespace sweet
{
    
namespace forge
{

class Target;
class TargetPrototype;
class Forge;
class LuaFileSystem;
class LuaContext;
class LuaGraph;
class LuaSystem;
class LuaTarget;
class LuaTargetPrototype;

class LuaForge
{
    Forge* forge_;
    lua_State* lua_state_;
    LuaFileSystem* lua_file_system_;
    LuaContext* lua_context_;
    LuaGraph* lua_graph_;
    LuaSystem* lua_system_;
    LuaTarget* lua_target_;
    LuaTargetPrototype* lua_target_prototype_;

public:
    LuaForge( Forge* forge );
    ~LuaForge();
    lua_State* lua_state() const;
    LuaTarget* lua_target() const;
    LuaTargetPrototype* lua_target_prototype() const;
    void create( Forge* forge );
    void destroy();
    void assign_global_variables( const std::vector<std::string>& assignments );

private:
    static int set_maximum_parallel_jobs( lua_State* lua_state );
    static int maximum_parallel_jobs( lua_State* lua_state );
    static int set_stack_trace_enabled( lua_State* lua_state );
    static int stack_trace_enabled( lua_State* lua_state );
    static int set_forge_hooks_library( lua_State* lua_state );
    static int forge_hooks_library( lua_State* lua_state );
    static int hash( lua_State* lua_state );
    static int execute( lua_State* lua_state );
    static int print( lua_State* lua_state );
    static lua_Integer hash_recursively( lua_State* lua_state, int table, bool hash_integer_keys );
};
    
}

}

#endif

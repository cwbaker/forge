#ifndef FORGE_LUA_HPP_INCLUDED
#define FORGE_LUA_HPP_INCLUDED

#include <vector>
#include <string>
#include <lua.hpp>

struct lua_State;

namespace sweet
{
    
namespace forge
{

class Target;
class Rule;
class Forge;
class LuaFileSystem;
class LuaContext;
class LuaGraph;
class LuaSystem;
class LuaTarget;
class LuaRule;
class LuaToolset;

class Lua
{
    Forge* forge_;
    lua_State* lua_state_;
    LuaFileSystem* lua_file_system_;
    LuaContext* lua_context_;
    LuaGraph* lua_graph_;
    LuaSystem* lua_system_;
    LuaTarget* lua_target_;
    LuaRule* lua_rule_;
    LuaToolset* lua_toolset_;

public:
    Lua( Forge* forge );
    ~Lua();
    lua_State* lua_state() const;
    LuaTarget* lua_target() const;
    LuaRule* lua_rule() const;
    LuaToolset* lua_toolset() const;
    void create( Forge* forge );
    void destroy();
    void assign_variables( const std::vector<std::string>& assignments );
    void set_package_path( const std::string& path );
};
    
}

}

#endif

#ifndef SWEET_BUILD_TOOL_LUATARGETPROTOTYPE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUATARGETPROTOTYPE_HPP_INCLUDED

namespace sweet
{

namespace lua
{

class LuaObject;
class Lua;

}

namespace build_tool
{

class LuaTarget;
class TargetPrototype;

class LuaTargetPrototype
{
    lua::Lua* lua_; ///< The main Lua virtual machine to create the target API in.
    LuaTarget* lua_target_;
    lua::LuaObject* target_prototype_metatable_; ///< The LuaObject that acts as a metatable for TargetPrototypes.
    lua::LuaObject* target_prototype_prototype_; ///< The LuaObject that acts as a prototype for TargetPrototypes.

public:
    LuaTargetPrototype();
    ~LuaTargetPrototype();
    void create( lua::Lua* lua, LuaTarget* lua_target );
    void destroy();
    void create_target_prototype( TargetPrototype* target_prototype );
    void destroy_target_prototype( TargetPrototype* target_prototype );
};
    
}

}

#endif

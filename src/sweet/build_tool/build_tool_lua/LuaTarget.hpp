#ifndef SWEET_BUILD_TOOL_LUATARGET_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUATARGET_HPP_INCLUDED

#include <ctime>

struct lua_State;

namespace sweet
{

namespace lua
{

class AddMember;
class LuaObject;
class Lua;

}

namespace build_tool
{

class TargetPrototype;
class Target;
class BuildTool;    

class LuaTarget
{
    lua::Lua* lua_; ///< The main Lua virtual machine to create the target API in.
    lua::LuaObject* target_metatable_; ///< The LuaObject that acts as a metatable for Targets.
    lua::LuaObject* target_prototype_; ///< The LuaObject that acts as a prototype for Targets.

public:
    LuaTarget();
    ~LuaTarget();
    void create( lua::Lua* lua );
    void destroy();

    void create_target( Target* target );
    void recover_target( Target* target );
    void update_target( Target* target );
    void destroy_target( Target* target );
    void register_functions( lua::AddMember& add_member );

private:
    Target* parent( Target* target );
    Target* target_working_directory( Target* target );
    Target* add_target( lua_State* lua_state );
    static int set_filename( lua_State* lua_state );
    static int filename( lua_State* lua_state );
    static int targets( lua_State* lua_state );
    static int dependency( lua_State* lua_state );
    static int dependencies_iterator( lua_State* lua_state );
    static int dependencies( lua_State* lua_state );
};

}

namespace lua
{

void lua_push( lua_State* lua, std::time_t timestamp );
std::time_t lua_to( lua_State* lua, int position, const std::time_t* null_pointer_for_overloading );

}

}

#endif
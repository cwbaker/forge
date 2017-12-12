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
    lua::LuaObject* target_prototype_; ///< The LuaObject that acts as a prototype for Targets.

public:
    LuaTarget();
    ~LuaTarget();

    lua::LuaObject* target_prototype() const;

    void create( lua::Lua* lua );
    void destroy();
    void create_target( Target* target );
    void recover_target( Target* target );
    void update_target( Target* target );
    void destroy_target( Target* target );

    static int id( lua_State* lua_state );
    static int path( lua_State* lua_state );
    static int branch( lua_State* lua_state );
    static int parent( lua_State* lua_state );
    static int prototype( lua_State* lua_state );
    static int set_cleanable( lua_State* lua_state );
    static int cleanable( lua_State* lua_state );
    static int set_built( lua_State* lua_state );
    static int built( lua_State* lua_state );
    static int timestamp( lua_State* lua_state );
    static int last_write_time( lua_State* lua_state );
    static int outdated( lua_State* lua_state );
    static int set_filename( lua_State* lua_state );
    static int filename( lua_State* lua_state );
    static int filenames( lua_State* lua_state );
    static int directory( lua_State* lua_state );
    static int set_working_directory( lua_State* lua_state );
    static int working_directory( lua_State* lua_state );
    static int add_explicit_dependency( lua_State* lua_state );
    static int remove_dependency( lua_State* lua_state );
    static int add_implicit_dependency( lua_State* lua_state );
    static int clear_implicit_dependencies( lua_State* lua_state );
    static int add_ordering_dependency( lua_State* lua_state );
    static int targets( lua_State* lua_state );
    static int any_dependency( lua_State* lua_state );
    static int any_dependencies_iterator( lua_State* lua_state );
    static int any_dependencies( lua_State* lua_state );
    static int explicit_dependency( lua_State* lua_state );
    static int explicit_dependencies_iterator( lua_State* lua_state );
    static int explicit_dependencies( lua_State* lua_state );
    static int implicit_dependency( lua_State* lua_state );
    static int implicit_dependencies_iterator( lua_State* lua_state );
    static int implicit_dependencies( lua_State* lua_state );
    static int ordering_dependency( lua_State* lua_state );
    static int ordering_dependencies_iterator( lua_State* lua_state );
    static int ordering_dependencies( lua_State* lua_state );
};

}

namespace lua
{

void lua_push( lua_State* lua, std::time_t timestamp );
std::time_t lua_to( lua_State* lua, int position, const std::time_t* null_pointer_for_overloading );

}

}

#endif
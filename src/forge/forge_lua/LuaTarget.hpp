#ifndef FORGE_LUATARGET_HPP_INCLUDED
#define FORGE_LUATARGET_HPP_INCLUDED

#include <ctime>

struct lua_State;

namespace sweet
{

namespace forge
{

class Target;
class Forge;    

class LuaTarget
{
    lua_State* lua_state_; ///< The main Lua virtual machine to create the target API in.

public:
    static const char* TARGET_METATABLE;

    LuaTarget();
    ~LuaTarget();

    void create( lua_State* lua_state, Forge* forge );
    void destroy();
    void create_target( Target* target );
    void update_target( Target* target );
    void destroy_target( Target* target );

    static int id( lua_State* lua_state );
    static int path( lua_State* lua_state );
    static int branch( lua_State* lua_state );
    static int parent( lua_State* lua_state );
    static int rule( lua_State* lua_state );
    static int set_cleanable( lua_State* lua_state );
    static int cleanable( lua_State* lua_state );
    static int set_built( lua_State* lua_state );
    static int built( lua_State* lua_state );
    static int timestamp( lua_State* lua_state );
    static int last_write_time( lua_State* lua_state );
    static int outdated( lua_State* lua_state );
    static int add_filename( lua_State* lua_state );
    static int set_filename( lua_State* lua_state );
    static int clear_filenames( lua_State* lua_state );
    static int filename( lua_State* lua_state );
    static int filenames_iterator( lua_State* lua_state );
    static int filenames( lua_State* lua_state );
    static int directory( lua_State* lua_state );
    static int set_working_directory( lua_State* lua_state );
    static int working_directory( lua_State* lua_state );
    static int add_explicit_dependency( lua_State* lua_state );
    static int remove_dependency( lua_State* lua_state );
    static int add_implicit_dependency( lua_State* lua_state );
    static int clear_implicit_dependencies( lua_State* lua_state );
    static int add_ordering_dependency( lua_State* lua_state );
    static int add_passive_dependency( lua_State* lua_state );
    static int all_dependencies_iterator( lua_State* lua_state );
    static int all_dependencies( lua_State* lua_state );
    static int explicit_dependency( lua_State* lua_state );
    static int explicit_dependencies_iterator( lua_State* lua_state );
    static int explicit_dependencies( lua_State* lua_state );
    static int ordering_dependency( lua_State* lua_state );
    static int vector_string_const_iterator_gc( lua_State* lua_state );
    static int target_call_metamethod( lua_State* lua_state );
    static int depend_call_metamethod( lua_State* lua_state );
};

}

}

#endif
#ifndef SWEET_FSYS_LUAFSYS_HPP_INCLUDED
#define SWEET_FSYS_LUAFSYS_HPP_INCLUDED

#include <sweet/fsys/BasicPath.hpp>

struct lua_State;

namespace sweet
{
    
namespace fsys
{

class FileSystem;

class LuaFsys
{
    lua_State* lua_state_;

public:
    LuaFsys();
    ~LuaFsys();
    void create( FileSystem* file_system, lua_State* lua_state );
    void create_with_existing_table( FileSystem* file_system, lua_State* lua_state );
    void destroy();
    void register_functions( FileSystem* file_system, lua_State* lua_state );

private:
    static int cd( lua_State* lua_state );
    static int pushd( lua_State* lua_state );
    static int popd( lua_State* lua_state );
    static int pwd( lua_State* lua_state );
    static int absolute( lua_State* lua_state );
    static int relative( lua_State* lua_state );
    static int root( lua_State* lua_state );
    static int initial( lua_State* lua_state );
    static int executable( lua_State* lua_state );
    static int home( lua_State* lua_state );
}; 

}

}

#endif

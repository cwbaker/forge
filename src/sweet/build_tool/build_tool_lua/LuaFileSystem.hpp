#ifndef SWEET_BUILD_TOOL_LUAFILESYSTEM_HPP_INCLUDED
#define SWEET_BUILD_TOOL_LUAFILESYSTEM_HPP_INCLUDED

#include <boost/filesystem.hpp>

struct lua_State;

namespace sweet
{
    
namespace build_tool
{

class BuildTool;

/**
// Provide Lua bindings to file system operations.
*/
class LuaFileSystem
{
    lua_State* lua_state_;

public:
    LuaFileSystem();
    ~LuaFileSystem();
    void create( BuildTool* build_tool, lua_State* lua_state );
    void destroy();

private:
    static int exists( lua_State* lua_state );
    static int is_file( lua_State* lua_state );
    static int is_directory( lua_State* lua_state );
    static int ls( lua_State* lua_state );
    static int find( lua_State* lua_state );
    static int mkdir( lua_State* lua_state );
    static int cpdir( lua_State* lua_state );
    static int rmdir( lua_State* lua_state );
    static int cp( lua_State* lua_state );
    static int rm( lua_State* lua_state );

    static int ls_iterator( lua_State* lua_state );
    static void push_directory_iterator( lua_State* lua_state, const boost::filesystem::directory_iterator& iterator );
    static boost::filesystem::directory_iterator* to_directory_iterator( lua_State* lua_state, int index );
    static int directory_iterator_gc( lua_State* lua_state );

    static int find_iterator( lua_State* lua_state );
    static void push_recursive_directory_iterator( lua_State* lua_state, const boost::filesystem::recursive_directory_iterator& iterator );
    static boost::filesystem::recursive_directory_iterator* to_recursive_directory_iterator( lua_State* lua_state, int index );
    static int recursive_directory_iterator_gc( lua_State* lua_state );

    static boost::filesystem::path absolute( lua_State* lua_state, int index );
}; 

}

}

#endif

#ifndef SWEET_FS_LUAFS_HPP_INCLUDED
#define SWEET_FS_LUAFS_HPP_INCLUDED

#include <sweet/fs/BasicPath.hpp>
#include <boost/filesystem.hpp>

struct lua_State;

namespace sweet
{
    
namespace fs
{

class FileSystem;

class LuaFs
{
    lua_State* lua_state_;

public:
    LuaFs();
    ~LuaFs();
    void create( FileSystem* file_system, lua_State* lua_state );
    void destroy();

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

    static int native( lua_State* lua_state );
    static int branch( lua_State* lua_state );
    static int leaf( lua_State* lua_state );
    static int basename( lua_State* lua_state );
    static int extension( lua_State* lua_state );
    static int is_absolute( lua_State* lua_state );
    static int is_relative( lua_State* lua_state );

    static int exists( lua_State* lua_state );
    static int is_file( lua_State* lua_state );
    static int is_directory( lua_State* lua_state );
    static int ls( lua_State* lua_state );
    static int find( lua_State* lua_state );
    static int mkdir( lua_State* lua_state );
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
}; 

}

}

#endif

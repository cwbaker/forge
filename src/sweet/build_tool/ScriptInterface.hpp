#ifndef SWEET_BUILD_TOOL_SCRIPTINTERFACE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_SCRIPTINTERFACE_HPP_INCLUDED

#include <sweet/lua/Lua.hpp>
#include <sweet/lua/LuaObject.hpp>
#include <sweet/path/Path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <vector>

namespace sweet
{

namespace build_tool
{

class Environment;
class OsInterface;
class Scanner;
class TargetPrototype;
class Arguments;
class Graph;
class Target;
class OsInterface;
class BuildTool;

/**
// Provide the API for Lua scripts.
*/
class ScriptInterface
{
    OsInterface* os_interface_; ///< The OsInterface to use to interface with the operating system.
    BuildTool* build_tool_; ///< The BuildTool that this ScriptInterface is part of.
    lua::Lua lua_; ///< The Lua virtual machine.
    lua::LuaObject target_prototype_metatable_; ///< The LuaObject that acts as a metatable for TargetPrototypes.
    lua::LuaObject target_prototype_prototype_; ///< The LuaObject that acts as a prototype for TargetPrototypes.
    lua::LuaObject scanner_metatable_; ///< The LuaObject that acts as a metatable for Scanners.
    lua::LuaObject scanner_prototype_; ///< The LuaObject that acts as a prototype for Scanners.
    lua::LuaObject target_metatable_; ///< The LuaObject that acts as a metatable for Targets.
    lua::LuaObject target_prototype_; ///< The LuaObject that acts as a prototype for Targets.
    std::vector<TargetPrototype*> target_prototypes_; ///< The TargetPrototypes that have been loaded in.
    std::vector<Scanner*> scanners_; ///< The Scanners that have been allocated.
    path::Path root_directory_; ///< The full path to the root directory.
    path::Path initial_directory_; ///< The full path to the initial directory.

    public:
        ScriptInterface( OsInterface* os_interface, BuildTool* build_tool );
        ~ScriptInterface();

        lua::Lua& lua();
        Environment* environment() const;

        void set_root_directory( const path::Path& root_directory );
        const path::Path& root_directory() const;

        void set_initial_directory( const path::Path& initial_directory );
        const path::Path& initial_directory() const;

        void create_prototype( TargetPrototype* target_prototype );
        void destroy_prototype( TargetPrototype* target_prototype );
        Scanner* create_scanner();
        void destroy_scanner( Scanner* scanner );
        void create_target( Target* target );
        void recover_target( Target* target );
        void update_target( Target* target, TargetPrototype* target_prototype );
        void destroy_target( Target* target );
        
        TargetPrototype* target_prototype( const std::string& id );
        Target* find_target( const std::string& path );
        std::string absolute( const std::string& path, const path::Path& working_directory = path::Path() );
        std::string relative( const std::string& path, const path::Path& working_directory = path::Path() );
        std::string root( const std::string& path ) const;
        std::string initial( const std::string& path ) const;
        std::string home( const std::string& path ) const;
        std::string anonymous() const;

        bool is_absolute( const std::string & path );
        bool is_relative( const std::string & path );
        void cd( const std::string& path );
        void pushd( const std::string& path );
        void popd();
        const std::string& pwd() const;
        Target* working_directory();
        std::string lower( const std::string& value );
        std::string upper( const std::string& value );
        std::string native( const std::string& path );
        std::string branch( const std::string& path );
        std::string leaf( const std::string& path );
        std::string basename( const std::string& path );
        std::string extension( const std::string& path );
        void print( const std::string& text );        
        bool exists( const std::string& path );
        bool is_file( const std::string& path );
        bool is_directory( const std::string& path );
        boost::filesystem::directory_iterator ls( const std::string& path );
        boost::filesystem::recursive_directory_iterator find( const std::string& path );
        void mkdir( const std::string& path );
        void cpdir( const std::string& from, const std::string& to );
        void rmdir( const std::string& path );
        void cp( const std::string& from, const std::string& to );
        void rm( const std::string& path );
        std::string operating_system();
        void putenv( const std::string& attribute, const std::string& value );
        const char* getenv( const char* name );
        void sleep( float milliseconds );
        float ticks();
        
        void buildfile( const std::string& filename, Target* target );
        int bind( Target* target );
        void mark_implicit_dependencies();
        void print_dependencies( Target* target );
        void print_namespace( Target* target );
        void wait();
        void clear();
        Target* load_xml( const std::string& filename );
        void save_xml();
        Target* load_binary( const std::string& filename );
        void save_binary();

    private:
        Target* parent( Target* target );
        Target* target_working_directory( Target* target );
        Target* add_target( lua_State* lua_state );
        static int target_prototype__( lua_State* lua_state );
        static int set_filename( lua_State* lua_state );
        static int filename( lua_State* lua_state );
        static int targets( lua_State* lua_state );
        static int dependency( lua_State* lua_state );
        static int dependencies( lua_State* lua_state );
        static int absolute_( lua_State* lua_state );
        static int relative_( lua_State* lua_state );
        static int root_( lua_State* lua_state );
        static int initial_( lua_State* lua_state );
        static int home_( lua_State* lua_state );
        static int getenv_( lua_State* lua_state );
        static int file( lua_State* lua_state );
        static int target( lua_State* lua_state );
        static int scanner( lua_State* lua_state );
        static int preorder( lua_State* lua_state );
        static int postorder( lua_State* lua_state );
        static int execute( lua_State* lua_state );
        static int scan( lua_State* lua_state );
};

}

}

SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::Scanner, LuaByReference );
SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::TargetPrototype, LuaByReference );
SWEET_LUA_TYPE_CONVERSION( sweet::build_tool::Target, LuaByReference );

#endif

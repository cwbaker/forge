#ifndef SWEET_BUILD_TOOL_OSINTERFACE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_OSINTERFACE_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/path/Path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <string>
#include <ctime>

namespace sweet
{

namespace build_tool
{

/**
// The interface from the %build tool library to the operating system.
*/
class OsInterface
{
    float initial_tick_count_; ///< The tick count when this OsInterface object was created.

    public:
        OsInterface();
        
        bool exists( const std::string& path );
        bool is_file( const std::string& path );
        bool is_directory( const std::string& path );
        bool is_regular( const std::string& path );
        std::time_t last_write_time( const std::string& path );
        boost::filesystem::directory_iterator ls( const std::string& path );
        boost::filesystem::recursive_directory_iterator find( const std::string& path );
        void mkdir( const std::string& path );
        void cpdir( const std::string& from, const std::string& to, const path::Path& base_path );
        void rmdir( const std::string& path );
        void cp( const std::string& from, const std::string& to );
        void rm( const std::string& path );
        std::string operating_system();
        void putenv( const std::string& attribute, const std::string& value );
        const char* getenv( const char* name );
        void sleep( float milliseconds );
        float ticks();
};

}

}

#endif
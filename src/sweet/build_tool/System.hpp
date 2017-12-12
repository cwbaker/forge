#ifndef SWEET_BUILD_TOOL_SYSTEM_HPP_INCLUDED
#define SWEET_BUILD_TOOL_SYSTEM_HPP_INCLUDED

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
class System
{
    float initial_tick_count_; ///< The tick count when this System object was created.

    public:
        System();
        
        bool exists( const std::string& path );
        bool is_file( const std::string& path );
        bool is_directory( const std::string& path );
        bool is_regular( const std::string& path );
        std::time_t last_write_time( const std::string& path );
        boost::filesystem::directory_iterator ls( const std::string& path );
        boost::filesystem::recursive_directory_iterator find( const std::string& path );
        std::string executable();
        std::string home();
        void mkdir( const std::string& path );
        void cpdir( const std::string& from, const std::string& to );
        void rmdir( const std::string& path );
        void cp( const std::string& from, const std::string& to );
        void rm( const std::string& path );
        const char* operating_system();
        const char* getenv( const char* name );
        void sleep( float milliseconds );
        float ticks();
};

}

}

#endif

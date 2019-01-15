#ifndef FORGE_SYSTEM_HPP_INCLUDED
#define FORGE_SYSTEM_HPP_INCLUDED

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <string>
#include <ctime>

namespace sweet
{

namespace forge
{

/**
// The interface from the %build tool library to the operating system.
*/
class System
{
    float initial_tick_count_; ///< The tick count when this System object was created.

    public:
        System();
        
        bool exists( const std::string& path ) const;
        bool is_file( const std::string& path ) const;
        bool is_directory( const std::string& path ) const;
        bool is_regular( const std::string& path ) const;
        std::time_t last_write_time( const std::string& path ) const;
        boost::filesystem::directory_iterator ls( const std::string& path ) const;
        boost::filesystem::recursive_directory_iterator find( const std::string& path ) const;
        std::string executable() const;
        std::string home() const;
        void mkdir( const std::string& path ) const;
        void rmdir( const std::string& path ) const;
        void cp( const std::string& from, const std::string& to ) const;
        void rm( const std::string& path ) const;
        const char* operating_system() const;
        const char* getenv( const char* name ) const;
        void sleep( float milliseconds ) const;
        float ticks() const;
};

}

}

#endif

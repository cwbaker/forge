//
// System.cpp
// Copyright (c) Charles Baker.` All rights reserved.
//

#include "System.hpp"
#include "Error.hpp"
#include <sweet/assert/assert.hpp>

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOS)
#include <unistd.h>
#include <time.h>
#include <mach-o/dyld.h>
#elif defined(BUILD_OS_LINUX)
#include <unistd.h>
#include <linux/limits.h>
#endif

using std::string;
using namespace sweet;
using namespace sweet::build_tool;

/**
// Constructor.
*/
System::System()
: initial_tick_count_( 0.0f )
{
    initial_tick_count_ = ticks();
}

/**
// Does a file exist?
//
// @param path
//  The path to the file to check for the existence of.
//
// @return
//  True if \e path exists otherwise false.
*/
bool System::exists( const std::string& path ) 
{
    return boost::filesystem::exists( path );
}

/**
// Is a file system entry a directory?
//
// @param path
//  The path to the file system entry to check for being a directory.
//
// @return
//  True if \e path is a directory otherwise false.
*/
bool System::is_file( const std::string& path )
{
    return boost::filesystem::is_regular( path );
}

/**
// Is a file system entry a directory?
//
// @param path
//  The path to the file system entry to check for being a directory.
//
// @return
//  True if \e path is a directory otherwise false.
*/
bool System::is_directory( const std::string& path )
{
    return boost::filesystem::is_directory( path );
}

/**
// Is a file system entry a regular file?
//
// @param path
//  The path to the file system entry to check for being regular.
//
// @return
//  True if \e path is a regular file otherwise false.
*/
bool System::is_regular( const std::string& path )
{
    return boost::filesystem::is_regular( path );
}

/**
// Get the last write time of the file system entry \e path.
//
// @param path
//  The path to the file system entry to get the last write time of.
//
// @return
//  The last write time of the file system entry \e path.
*/
std::time_t System::last_write_time( const std::string& path )
{
    return boost::filesystem::last_write_time( path );
}

/**
// List the files in a directory.
//
// @param path
//  The directory to list files in.
//
// @return
//  A boost::filesystem::directory_iterator that iterates over the files
//  in the directory.
*/
boost::filesystem::directory_iterator System::ls( const std::string& path )
{
    return boost::filesystem::exists( path ) ? 
        boost::filesystem::directory_iterator( path ) : 
        boost::filesystem::directory_iterator()
    ;
}

/**
// Recursively list the files in a directory and its children.
//
// @param path
//  The directory to list files in.
//
// @return
//  A boost::filesystem::recursive_directory_iterator that recursively 
//  iterates over the files in the directory and its children.
*/
boost::filesystem::recursive_directory_iterator System::find( const std::string& path )
{
    return boost::filesystem::exists( path ) ? 
        boost::filesystem::recursive_directory_iterator( path ) : 
        boost::filesystem::recursive_directory_iterator()
    ;
}

/**
// Get the full path to the build executable.
//
// @return
//  Returns the full path to the build executable.
*/
std::string System::executable()
{
#if defined(BUILD_OS_WINDOWS)
    char path [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( NULL, path, sizeof(path) );
    path [sizeof(path) - 1] = 0;
    return boost::filesystem::path( string(path, size) ).generic_string();
#elif defined(BUILD_OS_MACOS)
    uint32_t size = 0;
    _NSGetExecutablePath( NULL, &size );
    char executable_path [size];
    _NSGetExecutablePath( executable_path, &size );
    char linked_path [PATH_MAX];
    int linked_size = readlink( executable_path, linked_path, sizeof(linked_path) - 1 );
    if ( linked_size == -1 && errno == EINVAL )
    {
        return boost::filesystem::path( string(executable_path, size) ).generic_string();
    }
    else if ( linked_size >= 0 )
    {
        SWEET_ASSERT( linked_size >= 0 && linked_size < int(sizeof(linked_path)) );
        linked_path[linked_size] = 0;
        return boost::filesystem::path( string(linked_path, linked_size) ).generic_string();
    }
    return std::string();
#elif defined(BUILD_OS_LINUX)
    char path [PATH_MAX];
    ssize_t length = readlink( "/proc/self/exe", path, sizeof(path) );
    if ( length >= 0 )
    {
        return boost::filesystem::path( string(path, length) ).generic_string();
    }
    return string();
#else
    return string();
#endif
}

/**
// Get the full path to the user's home directory.
*/
std::string System::home()
{
#if defined (BUILD_OS_WINDOWS)
    const char* HOME = "USERPROFILE";
#elif defined (BUILD_OS_MACOS)
    const char* HOME = "HOME";
#elif defined (BUILD_OS_LINUX)
    const char* HOME = "HOME";
#else
#error "ScriptInterface::home() is not implemented for this platform"
#endif
    
    const char* home = ::getenv( HOME );
    return home ? boost::filesystem::path( string(home) ).generic_string() : string();
}

/**
// Make a directory and any intermediate directories that don't already exist.
//
// @param path
//  The path to the directory to create.
*/
void System::mkdir( const std::string& path )
{
    boost::filesystem::create_directories( path );
}

/**
// Recursively remove a directory and its contents.
//
// @param path
//  The directory to remove.
*/
void System::rmdir( const std::string& path )
{
    boost::filesystem::remove_all( path );
}

/**
// Copy a file or directory.
//
// @param from
//  The file to copy.
//
// @param to
//  The destination to copy the file to.
*/
void System::cp( const std::string& from, const std::string& to )
{
    boost::filesystem::copy_file( from, to );
}

/**
// Remove a file or directory.
//
// @param path
//  The path to the file or directory to remove.
*/
void System::rm( const std::string& path )
{
    boost::filesystem::remove( path );
}

/**
// Get a string that identifies the host operating system.
//
// @return
//  The string "windows" on Windows or the string "macosx" on MacOSX.
*/
const char* System::operating_system()
{
#if defined(BUILD_OS_WINDOWS)
    return "windows";
#elif defined(BUILD_OS_MACOS)
    return "macos";

#elif defined(BUILD_OS_LINUX)
    return "linux";
#else
#error "System::operating_system() is not implemented for this platform"
#endif
}

/**
// Get the value of an environment attribute.
//
// @param attribute
//  The name of the environment attribute to get the value of.
//
// @return
//  The value of the environment attribute or null if the environment attribute 
//  isn't set.
*/
const char* System::getenv( const char* name )
{
    SWEET_ASSERT( name );
    return ::getenv( name );
}

/**
// Pause execution.
//
// @param milliseconds
//  The number of milliseconds to pause execution for.
*/
void System::sleep( float milliseconds )
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( milliseconds >= 0.0f );
    ::Sleep( static_cast<DWORD>(milliseconds) );
#elif defined(BUILD_OS_MACOS)
    SWEET_ASSERT( false );
    const float MICROSECONDS_PER_MILLISECOND = 1000.0f;
    usleep( milliseconds * MICROSECONDS_PER_MILLISECOND );
#elif defined(BUILD_OS_LINUX)
    usleep( milliseconds * 1000.0f );
#else
#error "System::sleep() is not implemented for this platform"
#endif
}

/**
// Get the number of milliseconds elapsed since the start of the system.
//
// @return
//  The number of milliseconds elapsed since the system was started.
*/
float System::ticks()
{    
#if defined(BUILD_OS_WINDOWS)
    return static_cast<float>( ::GetTickCount() ) - initial_tick_count_;
#elif defined(BUILD_OS_MACOS)
    return static_cast<float>( (clock() - initial_tick_count_) * 1000 / CLOCKS_PER_SEC );
#elif defined(BUILD_OS_LINUX)
    return static_cast<float>( (clock() - initial_tick_count_) * 1000 / CLOCKS_PER_SEC );
#else
#error "System::ticks() is not implemented for this platform"
#endif
}

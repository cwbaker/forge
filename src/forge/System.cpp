//
// System.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "System.hpp"
#include <assert/assert.hpp>

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOS)
#include <unistd.h>
#include <time.h>
#include <mach-o/dyld.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(BUILD_OS_LINUX)
#include <unistd.h>
#include <linux/limits.h>
#include <sys/sysinfo.h>
#endif

using std::string;
using namespace sweet;
using namespace sweet::forge;

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
bool System::exists( const std::string& path ) const
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
bool System::is_file( const std::string& path ) const
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
bool System::is_directory( const std::string& path ) const
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
bool System::is_regular( const std::string& path ) const
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
std::time_t System::last_write_time( const std::string& path ) const
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
boost::filesystem::directory_iterator System::ls( const std::string& path ) const
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
boost::filesystem::recursive_directory_iterator System::find( const std::string& path ) const
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
std::string System::executable() const
{
#if defined(BUILD_OS_WINDOWS)
    char executable_path [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( nullptr, executable_path, sizeof(executable_path) );
    executable_path [sizeof(executable_path) - 1] = 0;
    HANDLE file = ::CreateFileA( executable_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
    if ( file != INVALID_HANDLE_VALUE )
    {
        char linked_path [MAX_PATH + 1];
        DWORD linked_size = ::GetFinalPathNameByHandleA( file, linked_path, sizeof(linked_path), VOLUME_NAME_DOS );
        ::CloseHandle( file );
        file = INVALID_HANDLE_VALUE;
        if ( linked_size < sizeof(linked_path) )
        {
            // The path returned by `::GetFinalPathNameByHandleA()` has a 
            // prefix of `\\?\` to indicate that it is an extended length 
            // path.  Skipping over it works for now but is probably the wrong
            // thing in many cases.
            const char* linked_path_without_extended_length_prefix = linked_path + 4;
            return boost::filesystem::path( string(linked_path_without_extended_length_prefix, linked_size - 4) ).generic_string();
        }
    }
    return std::string();
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
std::string System::home() const
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
void System::mkdir( const std::string& path ) const
{
    boost::filesystem::create_directories( path );
}

/**
// Recursively remove a directory and its contents.
//
// @param path
//  The directory to remove.
*/
void System::rmdir( const std::string& path ) const
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
void System::cp( const std::string& from, const std::string& to ) const
{
    boost::filesystem::copy_file( from, to );
}

/**
// Remove a file or directory.
//
// @param path
//  The path to the file or directory to remove.
*/
void System::rm( const std::string& path ) const
{
    boost::filesystem::remove( path );
}

/**
// Get a string that identifies the host operating system.
//
// @return
//  The string "windows" on Windows or the string "macosx" on MacOSX.
*/
const char* System::operating_system() const
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
const char* System::getenv( const char* name ) const
{
    SWEET_ASSERT( name );
    return ::getenv( name );
}

/**
// Get the number of logical processors available in the system.
//
// @return
//  The number of logical processors.
*/
int System::number_of_logical_processors() const
{
#if defined(BUILD_OS_WINDOWS)
    SYSTEM_INFO system_info = {0};
    ::GetSystemInfo( &system_info );
    return int(system_info.dwNumberOfProcessors);
#elif defined(BUILD_OS_MACOS)
    int processors = 0;
    size_t length = sizeof(processors);
    sysctlbyname( "hw.logicalcpu", &processors, &length, nullptr, 0 );
    return processors;
#elif defined(BUILD_OS_LINUX)
    return get_nprocs();
#else
#error "System::number_of_logical_processors() is not implemented for this platform"
#endif
}

/**
// Pause execution.
//
// @param milliseconds
//  The number of milliseconds to pause execution for.
*/
void System::sleep( float milliseconds ) const
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
float System::ticks() const
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

//
// OsInterface.cpp
// Copyright (c) 2010 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "OsInterface.hpp"
#include "Error.hpp"

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOSX)
#include <unistd.h>
#include <time.h>
#endif

using namespace sweet;
using namespace sweet::build_tool;

/**
// Constructor.
*/
OsInterface::OsInterface()
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
bool OsInterface::exists( const std::string& path ) 
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
bool OsInterface::is_file( const std::string& path )
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
bool OsInterface::is_directory( const std::string& path )
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
bool OsInterface::is_regular( const std::string& path )
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
std::time_t OsInterface::last_write_time( const std::string& path )
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
boost::filesystem::directory_iterator OsInterface::ls( const std::string& path )
{
    return boost::filesystem::exists(path) ? boost::filesystem::directory_iterator( path ) : boost::filesystem::directory_iterator();
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
boost::filesystem::recursive_directory_iterator OsInterface::find( const std::string& path )
{
    return boost::filesystem::exists(path) ? boost::filesystem::recursive_directory_iterator( path ) : boost::filesystem::recursive_directory_iterator();
}

/**
// Make a directory and any intermediate directories that don't already exist.
//
// @param path
//  The path to the directory to create.
*/
void OsInterface::mkdir( const std::string& path )
{
    boost::filesystem::create_directories( path );
}

/**
// Recursively copy a directory and its contents to another directory only
// copying newer files.
//
// Directories and files that start with a '.' aren't recursed into or 
// copied so that directory hierarchies beneath .svn can be ignored.
//
// @param from
//  The directory to copy from.
//
// @param to
//  The directory to copy to.
*/
void OsInterface::cpdir( const std::string& from, const std::string& to, const path::Path& base_path )
{
    using namespace boost::filesystem;

    path::Path from_path( from );
    path::Path to_path( to );

    if ( exists(from_path.string()) )
    {
        recursive_directory_iterator i = recursive_directory_iterator( from_path.string() );
        recursive_directory_iterator end;

        while ( i != end )
        {
            SWEET_ASSERT( i != end );
            const boost::filesystem::path& source = i->path();
            if ( !source.empty() && source.filename().string().at(0) != '.' )
            {
                path::Path destination = to_path / from_path.relative( source.string() );
                if ( is_directory(source.string()) )
                {
                    if ( !exists(destination.string()) )
                    {
                        create_directories( destination.string() );
                    }
                }
                else if ( !exists(destination.string()) || last_write_time(source.string()) > last_write_time(destination.string()) )
                {
                    boost::filesystem::path destination_directory = destination.branch().string();
                    if ( !exists(destination_directory.string()) )
                    {
                        create_directories( destination_directory );
                    }

                    if ( exists(destination.string()) )
                    {
                        remove( destination.string() );
                    }

                    copy_file( *i, destination.string() );
                }
            }
            else
            {
                i.no_push();
            }

            ++i;
        }
    }
}

/**
// Recursively remove a directory and its contents.
//
// @param path
//  The directory to remove.
*/
void OsInterface::rmdir( const std::string& path )
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
void OsInterface::cp( const std::string& from, const std::string& to )
{
    boost::filesystem::copy_file( from, to );
}

/**
// Remove a file or directory.
//
// @param path
//  The path to the file or directory to remove.
*/
void OsInterface::rm( const std::string& path )
{
    boost::filesystem::remove( path );
}

/**
// Get a string that identifies the host operating system.
//
// @return
//  The string "windows" on Windows or the string "macosx" on MacOSX.
*/
std::string OsInterface::operating_system()
{
#if defined(BUILD_OS_WINDOWS)
    return "windows";
#elif defined(BUILD_OS_MACOSX)
    return "macosx";
#else
#error "OsInterface::operating_system() is not implemented for this platform"
#endif
}

/**
// Set the value of an environment attribute.
//
// @param attribute
//  The environment attribute to set the value of.
//
// @param value
//  The value to set the environment attribute to or an empty string to unset
//  the environment attribute.
*/
void OsInterface::putenv( const std::string& attribute, const std::string& value )
{
    std::string assignment = attribute + "=" + value;
    ::putenv( strdup(const_cast<char*>(assignment.c_str())) );
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
const char* OsInterface::getenv( const char* name )
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
void OsInterface::sleep( float milliseconds )
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( milliseconds >= 0.0f );
    ::Sleep( static_cast<DWORD>(milliseconds) );
#elif defined(BUILD_OS_MACOSX)
    SWEET_ASSERT( false );
    const float MICROSECONDS_PER_MILLISECOND = 1000.0f;
    usleep( milliseconds * MICROSECONDS_PER_MILLISECOND );
#else
#error "OsInterface::sleep() is not implemented for this platform"
#endif
}

/**
// Get the number of milliseconds elapsed since the start of the system.
//
// @return
//  The number of milliseconds elapsed since the system was started.
*/
float OsInterface::ticks()
{    
#if defined(BUILD_OS_WINDOWS)
    return static_cast<float>( ::GetTickCount() ) - initial_tick_count_;
#elif defined(BUILD_OS_MACOSX)
    return static_cast<float>( (clock() - initial_tick_count_) * 1000 / CLOCKS_PER_SEC );
#else
#error "OsInterface::ticks() is not implemented for this platform"
#endif
}

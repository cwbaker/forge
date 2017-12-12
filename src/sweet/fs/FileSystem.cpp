//
// FileSystem.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "FileSystem.hpp"
#include "BasicPath.hpp"
#include "DirectoryStack.hpp"
#include "BasicPath.ipp"
#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOSX)
#include <unistd.h>
#include <time.h>
#include <mach-o/dyld.h>
#endif

using namespace sweet;
using namespace sweet::fs;

FileSystem::FileSystem()
: empty_(),
  root_(),
  initial_(),
  executable_(),
  home_(),
  directory_stack_( NULL )
{
    reset_executable();
    reset_home();
}

FileSystem::FileSystem( DirectoryStack* directory_stack )
: empty_(),
  root_(),
  initial_(),
  executable_(),
  home_(),
  directory_stack_( directory_stack )
{
    reset_executable();
    reset_home();
}

const fs::Path& FileSystem::root() const
{
    return root_;
}

const fs::Path& FileSystem::initial() const
{
    return initial_;
}

const fs::Path& FileSystem::executable() const
{
    return executable_;
}

const fs::Path& FileSystem::home() const
{
    return home_;
}

fs::Path FileSystem::root( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( root_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path FileSystem::initial( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( initial_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path FileSystem::executable( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( executable_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

fs::Path FileSystem::home( const fs::Path& path ) const
{
    if ( fs::Path(path).is_absolute() )
    {
        return path;
    }

    fs::Path absolute_path( home_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

void FileSystem::reset_root( const fs::Path& root )
{
    root_ = root;
}

void FileSystem::reset_initial( const fs::Path& initial )
{
    initial_ = initial;
}

void FileSystem::reset_executable()
{
#if defined(BUILD_OS_WINDOWS)
    char path [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( NULL, path, sizeof(path) );
    path [sizeof(path) - 1] = 0;
    executable_ = fs::Path( path );
#elif defined(BUILD_OS_MACOSX)
    uint32_t size = 0;
    _NSGetExecutablePath( NULL, &size );
    char path [size];
    _NSGetExecutablePath( path, &size );
    executable_ = fs::Path( path );
#endif
}

void FileSystem::reset_home()
{
#if defined(BUILD_OS_WINDOWS)
    char path [MAX_PATH + 1];
    int size = ::GetModuleFileNameA( NULL, path, sizeof(path) );
    path [sizeof(path) - 1] = 0;
    home_ = fs::Path( path );
#elif defined(BUILD_OS_MACOSX)
    uint32_t size = 0;
    _NSGetExecutablePath( NULL, &size );
    char path [size];
    _NSGetExecutablePath( path, &size );
    home_ = fs::Path( path );
#endif
}

DirectoryStack* FileSystem::directory_stack() const
{
    return directory_stack_;
}

const fs::Path& FileSystem::directory() const
{
    return directory_stack_ ? directory_stack_->directory() : empty_;
}

void FileSystem::set_directory_stack( DirectoryStack* directory_stack )
{
    directory_stack_ = directory_stack;
}

void FileSystem::reset_directory( const fs::Path& directory )
{
    if ( directory_stack_ ) 
    {
        directory_stack_->reset_directory( directory );
    }
}

void FileSystem::change_directory( const fs::Path& directory )
{
    if ( directory_stack_ ) 
    {
        directory_stack_->change_directory( directory );
    }
}

void FileSystem::push_directory( const fs::Path& directory )
{
    if ( directory_stack_ ) 
    {
        directory_stack_->push_directory( directory );
    }
}

void FileSystem::pop_directory()
{
    if ( directory_stack_ ) 
    {
        directory_stack_->pop_directory();
    }
}

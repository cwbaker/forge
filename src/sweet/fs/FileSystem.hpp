#ifndef SWEET_FS_FILESYSTEM_HPP_INCLUDED
#define SWEET_FS_FILESYSTEM_HPP_INCLUDED

#include "BasicPath.hpp"

namespace sweet
{

namespace fs
{

class DirectoryStack;

class FileSystem
{
    fs::Path empty_; ///< An empty path to return for DirectoryStack calls when DirectoryStack is null.
    fs::Path root_; ///< The full path to the root directory.
    fs::Path initial_; ///< The full path to the initial directory.
    fs::Path executable_; ///< The full path to the build executable directory.
    fs::Path home_; ///< The full path to the user's home directory.
    DirectoryStack* directory_stack_; ///< The stack of directories managed by cd, pushd, popd, etc.

public:
    FileSystem();
    FileSystem( DirectoryStack* directory_stack );

    const fs::Path& root() const;
    const fs::Path& initial() const;
    const fs::Path& executable() const;
    const fs::Path& home() const;

    fs::Path root( const fs::Path& path ) const;
    fs::Path initial( const fs::Path& path ) const;
    fs::Path executable( const fs::Path& path ) const;
    fs::Path home( const fs::Path& path ) const;

    void reset_root( const fs::Path& root );
    void reset_initial( const fs::Path& initial );
    void reset_executable();
    void reset_home();

    DirectoryStack* directory_stack() const;
    const fs::Path& directory() const;
    void set_directory_stack( DirectoryStack* directory_stack );
    void reset_directory( const fs::Path& directory );
    void change_directory( const fs::Path& directory );
    void push_directory( const fs::Path& directory );
    void pop_directory();
};

}

}

#endif

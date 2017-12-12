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
    fs::Path root_; ///< The full path to the root directory.
    fs::Path initial_; ///< The full path to the initial directory.
    fs::Path executable_; ///< The full path to the build executable directory.
    fs::Path home_; ///< The full path to the user's home directory.
    DirectoryStack* directory_stack_; ///< The stack of directories managed by cd, pushd, popd, etc.

public:
    FileSystem();

    const fs::Path& root() const;
    const fs::Path& initial() const;
    const fs::Path& executable() const;
    const fs::Path& home() const;

    fs::Path root( const fs::Path& path ) const;
    fs::Path initial( const fs::Path& path ) const;
    fs::Path executable( const fs::Path& path ) const;
    fs::Path home( const fs::Path& path ) const;

    void set_root( const fs::Path& root );
    void set_initial( const fs::Path& initial );
    void refresh_executable();
    void refresh_home();

    void set_directory_stack( DirectoryStack* directory_stack );
    DirectoryStack* directory_stack() const;
};

}

}

#endif

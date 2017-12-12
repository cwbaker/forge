#ifndef SWEET_FSYS_FILESYSTEM_HPP_INCLUDED
#define SWEET_FSYS_FILESYSTEM_HPP_INCLUDED

#include "BasicPath.hpp"

namespace sweet
{

namespace fsys
{

class DirectoryStack;

class FileSystem
{
    fsys::Path root_; ///< The full path to the root directory.
    fsys::Path initial_; ///< The full path to the initial directory.
    fsys::Path executable_; ///< The full path to the build executable directory.
    fsys::Path home_; ///< The full path to the user's home directory.
    DirectoryStack* directory_stack_; ///< The stack of directories managed by cd, pushd, popd, etc.

public:
    FileSystem();

    const fsys::Path& root() const;
    const fsys::Path& initial() const;
    const fsys::Path& executable() const;
    const fsys::Path& home() const;

    fsys::Path root( const fsys::Path& path ) const;
    fsys::Path initial( const fsys::Path& path ) const;
    fsys::Path executable( const fsys::Path& path ) const;
    fsys::Path home( const fsys::Path& path ) const;

    void set_root( const fsys::Path& root );
    void set_initial( const fsys::Path& initial );
    void refresh_executable();
    void refresh_home();

    void set_directory_stack( DirectoryStack* directory_stack );
    DirectoryStack* directory_stack() const;
};

}

}

#endif

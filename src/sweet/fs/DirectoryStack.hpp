#ifndef SWEET_FS_DIRECTORYSTACK_HPP_INCLUDED
#define SWEET_FS_DIRECTORYSTACK_HPP_INCLUDED

#include "BasicPath.hpp"
#include <vector>

namespace sweet
{

namespace fs
{

/**
// A stack of directories with the top as the current working directory.
*/
class DirectoryStack
{
    std::vector<Path> directories_; ///< The stack of working directories (the element at the top is the current working directory).

public:
    DirectoryStack();
    const fs::Path& directory() const;
    void reset_directory( const fs::Path& directory );
    void change_directory( const fs::Path& directory );
    void push_directory( const fs::Path& directory );
    void pop_directory();
};

}

}

#endif

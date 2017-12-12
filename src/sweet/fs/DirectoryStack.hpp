#ifndef SWEET_FSYS_DIRECTORYSTACK_HPP_INCLUDED
#define SWEET_FSYS_DIRECTORYSTACK_HPP_INCLUDED

#include "BasicPath.hpp"
#include <vector>

namespace sweet
{

namespace fsys
{

/**
// A stack of directories with the top as the current working directory.
*/
class DirectoryStack
{
    std::vector<Path> directories_; ///< The stack of working directories (the element at the top is the current working directory).

public:
    DirectoryStack();
    const fsys::Path& directory() const;
    void reset_directory( const fsys::Path& directory );
    void change_directory( const fsys::Path& directory );
    void push_directory( const fsys::Path& directory );
    void pop_directory();
};

}

}

#endif

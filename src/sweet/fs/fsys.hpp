#ifndef SWEET_FSYS_FSYS_HPP_INCLUDED
#define SWEET_FSYS_FSYS_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/build.hpp>
#include <cctype>
#include <algorithm>
#include <functional>
#include <vector>
#include <sweet/assert/assert.hpp>
#include "BasicPathTraits.hpp"
#include "BasicPath.hpp"
#include "BasicPath.ipp"

#if !defined BUILD_MODULE_FSYS && defined _MSC_VER
#pragma comment( lib, "fsys" BUILD_LIBRARY_SUFFIX )
#endif

namespace sweet
{

/**
File System library.

Provides portable paths similar to those provided by boost::filesystem and 
some basic file system operations (copy, remove, exists, etc).
*/
namespace fsys
{

fsys::Path absolute( const fsys::Path& path, const fsys::Path& base_path );
fsys::Path relative( const fsys::Path& path, const fsys::Path& base_path );
std::string executable( const std::string& path );
std::string home( const std::string& path );

}

}

#endif

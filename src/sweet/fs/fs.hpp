#ifndef SWEET_FS_FS_HPP_INCLUDED
#define SWEET_FS_FS_HPP_INCLUDED

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

#if !defined BUILD_MODULE_FS && defined _MSC_VER
#pragma comment( lib, "fs" BUILD_LIBRARY_SUFFIX )
#endif

namespace sweet
{

/**
File System library.

Provides portable paths similar to those provided by boost::filesystem and 
some basic file system operations (copy, remove, exists, etc).
*/
namespace fs
{

Path absolute( const Path& path, const Path& base_path );
Path relative( const Path& path, const Path& base_path );
std::string executable( const std::string& path );
std::string home( const std::string& path );

}

}

#endif

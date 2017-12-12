#ifndef SWEET_PATH_PATH_HPP_INCLUDED
#define SWEET_PATH_PATH_HPP_INCLUDED

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

#if !defined BUILD_MODULE_PATH && defined _MSC_VER
#pragma comment( lib, "path" BUILD_LIBRARY_SUFFIX )
#endif

namespace sweet
{

/**
 %Path library.

 Provides portable paths similar to those provided by boost::filesystem.

 This component was initially created because the boost::filesystem::path
 didn't support wide character paths.  Boost 1.34.0 has 
 boost::filesystem::basic_path<> that can be templated on character type 
 will probably replace this component at some stage in the near future.

 Although the Boost implementation doesn't support the relative and 
 normalize operations that are quite useful.
*/
namespace path
{

std::string executable( const std::string& path );
std::string home( const std::string& path );

}

}

#endif

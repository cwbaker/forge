//
// path.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PATH_PATH_HPP_INCLUDED
#define SWEET_PATH_PATH_HPP_INCLUDED

#if defined(BUILD_MODULE_PATH) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PATH_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PATH_DECLSPEC __declspec(dllimport)
#else
#define SWEET_PATH_DECLSPEC
#endif 

#include <sweet/build.hpp>
#include <cctype>
#include <algorithm>
#include <functional>
#include <vector>
#include <sweet/assert/assert.hpp>
#include "BasicPathTraits.hpp"
#include "BasicPath.hpp"
#include "BasicPath.ipp"

#ifndef BUILD_MODULE_PATH
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
}

}

#endif

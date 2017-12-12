//
// declspec.hpp
// Copyright (c) 2010 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_DECLSPEC_HPP_INCLUDED
#define SWEET_BUILD_TOOL_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_BUILD_TOOL) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_BUILD_TOOL_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_BUILD_TOOL_DECLSPEC __declspec(dllimport)
#else
#define SWEET_BUILD_TOOL_DECLSPEC
#endif 

#endif

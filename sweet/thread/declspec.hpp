//
// declspec.hpp
// Copyright (c) 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_DECLSPEC_HPP_INCLUDED
#define SWEET_THREAD_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_THREAD) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_THREAD_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_THREAD_DECLSPEC __declspec(dllimport)
#else
#define SWEET_THREAD_DECLSPEC
#endif 

#endif
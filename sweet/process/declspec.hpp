//
// declspec.hpp
// Copyright (c) 2010 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PROCESS_DECLSPEC_HPP_INCLUDED
#define SWEET_PROCESS_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_PROCESS) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PROCESS_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PROCESS_DECLSPEC __declspec(dllimport)
#else
#define SWEET_PROCESS_DECLSPEC
#endif 

#endif

//
// declspec.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_DECLSPEC_HPP_INCLUDED
#define SWEET_PERSIST_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_PERSIST) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PERSIST_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PERSIST_DECLSPEC __declspec(dllimport)
#else
#define SWEET_PERSIST_DECLSPEC
#endif 

#endif

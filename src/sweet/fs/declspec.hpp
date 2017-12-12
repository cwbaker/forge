#ifndef SWEET_FSYS_DECLSPEC_HPP_INCLUDED
#define SWEET_FSYS_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_FSYS) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_FSYS_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_FSYS_DECLSPEC __declspec(dllimport)
#else
#define SWEET_FSYS_DECLSPEC
#endif 

#endif

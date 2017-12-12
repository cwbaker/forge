#ifndef SWEET_FS_DECLSPEC_HPP_INCLUDED
#define SWEET_FS_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_FS) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_FS_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_FS_DECLSPEC __declspec(dllimport)
#else
#define SWEET_FS_DECLSPEC
#endif 

#endif

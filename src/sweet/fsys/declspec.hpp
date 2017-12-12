#ifndef SWEET_PATH_DECLSPEC_HPP_INCLUDED
#define SWEET_PATH_DECLSPEC_HPP_INCLUDED

#if defined(BUILD_MODULE_PATH) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PATH_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define SWEET_PATH_DECLSPEC __declspec(dllimport)
#else
#define SWEET_PATH_DECLSPEC
#endif 

#endif

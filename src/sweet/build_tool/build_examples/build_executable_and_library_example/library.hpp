
#ifndef LIBRARY_HPP_INCLUDED
#define LIBRARY_HPP_INCLUDED

#if defined(BUILD_MODULE_LIBRARY) && defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define LIBRARY_DECLSPEC __declspec(dllexport)
#elif defined(BUILD_LIBRARY_TYPE_DYNAMIC)
#define LIBRARY_DECLSPEC __declspec(dllimport)
#else
#define LIBRARY_DECLSPEC
#endif 

extern LIBRARY_DECLSPEC void hello_world();

#endif

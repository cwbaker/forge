#ifndef SWEET_BUILD_HPP_INCLUDED
#define SWEET_BUILD_HPP_INCLUDED

// Define macros for target platform based on macros defined automatically
// by compilers when building to target those platforms.
#if _WIN32 || _WIN64
#define BUILD_OS_WINDOWS
#define BUILD_PLATFORM_WINDOWS
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE 
#define BUILD_OS_IOS
#define BUILD_PLATFORM_IOS
#elif TARGET_OS_MAC
#define BUILD_OS_MACOS
#define BUILD_PLATFORM_MACOS
#else
#error "Unknown Apple platform!"
#endif
#elif __ANDROID__
#define BUILD_OS_ANDROID
#define BUILD_OS_ANDROID
#define BUILD_PLATFORM_ANDROID
#elif __linux__
#define BUILD_OS_LINUX
#define BUILD_PLATFORM_LINUX
#endif

//
// Define macros for all components based on the variant that is being 
// built.
//
#if defined(BUILD_VARIANT_DEBUG)
#define SWEET_ASSERT_ENABLED
#define SWEET_EXCEPTIONS_ENABLED
#define SWEET_RTTI_ENABLED
#define SWEET_MEMORY_ENABLED
#define SWEET_MEMORY_TRACING_ENABLED
#define SWEET_POINTER_THREAD_SAFETY_ENABLED

#elif defined(BUILD_VARIANT_DEBUG_DLL)
#define SWEET_ASSERT_ENABLED
#define SWEET_EXCEPTIONS_ENABLED
#define SWEET_RTTI_ENABLED
#define SWEET_POINTER_THREAD_SAFETY_ENABLED

#elif defined(BUILD_VARIANT_RELEASE)
#define SWEET_ASSERT_ENABLED
#define SWEET_EXCEPTIONS_ENABLED
#define SWEET_RTTI_ENABLED
#define SWEET_MEMORY_ENABLED
#define SWEET_POINTER_THREAD_SAFETY_ENABLED

#elif defined(BUILD_VARIANT_RELEASE_DLL)
#define SWEET_ASSERT_ENABLED
#define SWEET_EXCEPTIONS_ENABLED
#define SWEET_RTTI_ENABLED
#define SWEET_POINTER_THREAD_SAFETY_ENABLED

#elif defined(BUILD_VARIANT_SHIPPING)
#define SWEET_DEBUG_DUMP_ENABLED
#define SWEET_EXCEPTIONS_ENABLED
#define SWEET_RTTI_ENABLED
#define SWEET_MEMORY_ENABLED
#define SWEET_POINTER_THREAD_SAFETY_ENABLED

#elif defined(BUILD_VARIANT_SHIPPING_DLL)
#define SWEET_DEBUG_DUMP_ENABLED
#define SWEET_EXCEPTIONS_ENABLED
#define SWEET_RTTI_ENABLED
#define SWEET_POINTER_THREAD_SAFETY_ENABLED

#endif

#endif

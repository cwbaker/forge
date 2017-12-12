
#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN

#include <boost/filesystem/operations.hpp>

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#endif

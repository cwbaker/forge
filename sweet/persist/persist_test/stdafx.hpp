
#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#define NOMINMAX

#include <sweet/build.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <typeinfo>
#include <float.h>

#include <exception>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <stack>
#include <algorithm>
#include <iostream>
#include <fstream>

#if defined(BUILD_OS_WINDOWS)
#include <hash_set>
#include <hash_map>
#endif

#include <sweet/assert/assert.hpp>
#include <sweet/error/error.hpp>

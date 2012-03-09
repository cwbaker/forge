#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#define NOMINMAX

#include <sweet/build.hpp>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <typeinfo.h>

#include <exception>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <list>
#include <set>
#include <hash_set>
#include <map>
#include <hash_map>

#include <sweet/assert/assert.hpp>
#include <sweet/error/error.hpp>
#include <sweet/rtti/rtti.hpp>
#include <sweet/path/path.hpp>

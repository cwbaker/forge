//
// strnlen.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "declspec.hpp"
#include "strnlen.hpp"

using namespace sweet::persist;

namespace sweet
{

namespace persist
{

size_t strnlen( const char* string, size_t max )
{
    SWEET_ASSERT( string );

    const char* pos = string;
    const char* end = string + max;
    while ( pos < end && *pos != '\0' )
    {
        ++pos;
    }
    return pos - string;
}

size_t strnlen( const wchar_t* string, size_t max )
{
    SWEET_ASSERT( string );

    const wchar_t* pos = string;
    const wchar_t* end = string + max;
    while ( pos < end && *pos != L'\0' )
    {
        ++pos;
    }
    return pos - string;
}

}

}

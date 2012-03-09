//
// strnlen.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "strnlen.hpp"
#include <sweet/assert/assert.hpp>

using namespace sweet::persist;

namespace sweet
{

namespace persist
{

unsigned int strnlen( const char* string, unsigned int max )
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

unsigned int strnlen( const wchar_t* string, unsigned int max )
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

//
// narrow.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "functions.hpp"
#include <vector>

using namespace sweet::persist;

//
// Disable the warning about unsafe use of std::ctype::narrow in the following
// function.
//
#pragma warning( push )
#pragma warning( disable: 4996 )

namespace sweet
{

namespace persist
{

std::string narrow( const std::wstring& wide_string, char unknown, const std::locale& locale )
{
    if ( wide_string.empty() )
    {
        return std::string();
    }

    size_t length = wide_string.length();
    std::vector<char> buffer( length );    
    const std::ctype<wchar_t>* ctype = &std::use_facet<std::ctype<wchar_t> >( locale );
    ctype->narrow( wide_string.c_str(), wide_string.c_str() + length, unknown, &buffer[0] );
    return std::string( &buffer[0], length );    
}

}

}

#pragma warning( pop )

//
// widen.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "functions.hpp"
#include <sweet/assert/assert.hpp>
#include <vector>

using namespace sweet::persist;

//
// Disable the warning about unsafe use of std::ctype::narrow in the following
// two functions.
//
#pragma warning( push )
#pragma warning( disable: 4996 )

namespace sweet
{

namespace persist
{

std::wstring widen( const std::string& narrow_string, const std::locale& locale )
{
    if ( narrow_string.empty() )
    {
        return std::wstring();
    }

    size_t length = narrow_string.length();
    std::vector<wchar_t> buffer( length );    
    const std::ctype<wchar_t>* ctype = &std::use_facet<std::ctype<wchar_t> >( locale );
    ctype->widen( narrow_string.c_str(), narrow_string.c_str() + length, &buffer[0] );
    return std::wstring( &buffer[0], length );    
}

std::wstring widen( const wchar_t* wide_string, const std::locale& locale )
{
    SWEET_ASSERT( wide_string );
    return std::wstring( wide_string );
}

}

}

#pragma warning( pop )

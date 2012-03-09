//
// EnumFilter.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "EnumFilter.hpp"
#include "Error.hpp"
#include "types.hpp"
#include <stdlib.h>

using namespace sweet::persist;

const sweet::persist::EnumFilter::Conversion sweet::persist::BASIC_TYPES[] =
{
    { TYPE_VOID, "void" },
    { TYPE_ADDRESS, "address" },
    { TYPE_BOOLEAN, "boolean" },
    { TYPE_INTEGER, "integer" },
    { TYPE_UNSIGNED_INTEGER, "unsigned_integer" },
    { TYPE_REAL, "real" },
    { TYPE_NUMBER, "number" },
    { TYPE_STRING, "string" },
    { TYPE_OBJECT, "object" },
    { -1, "void" }
};

EnumFilter::EnumFilter( const Conversion* conversions )
: m_conversions( conversions ),
  m_string()
{
}

int EnumFilter::to_memory( const std::string& value ) const
{
    const Conversion* conversion = m_conversions;
    while ( conversion->m_name != 0 && value != conversion->m_name )
    {
        ++conversion;
    }

    if ( conversion->m_name == 0 )
    {
        if ( value.empty() || isalpha(value[0]) )
        {
            SWEET_ERROR( InvalidIdentifierError("Unable to find enumerated value for '%s'", value.empty() ? "" : value.c_str()) );
        }

        return atoi( value.c_str() );
    }

    return conversion->m_value;
}

const std::string& EnumFilter::to_archive( int value ) const
{
    const Conversion* conversion = m_conversions;
    while ( conversion->m_name != 0 && conversion->m_value != value )
    {
        ++conversion;
    }

    if ( conversion->m_name != 0 )
    {
        m_string.assign( conversion->m_name );
    }
    else
    {
        char number [11];
        m_string.assign( _itoa(value, number, 10) );
    }

    return m_string;
}

EnumFilter sweet::persist::enum_filter( const EnumFilter::Conversion* conversions )
{
    return EnumFilter( conversions );
}

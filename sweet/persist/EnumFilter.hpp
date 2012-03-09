//
// EnumFilter.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_ENUMFILTER_HPP_INCLUDED
#define SWEET_PERSIST_ENUMFILTER_HPP_INCLUDED

#include "declspec.hpp"
#include <string>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// A filter that uses an array of structures to convert from an integer 
// value into a string.
//
// If no conversion can be found when converting to a string then the integer
// value is simply written out.
//
// If no conversion can be found when converting to an integer from a string
// value that begins with a digit then atoi() is used to convert the value to
// an integer.  If the string begins with a letter and there is no matching
// name then an exception is thrown.
*/
class SWEET_PERSIST_DECLSPEC EnumFilter
{
    public:
        struct Conversion
        {
            int m_value;
            const char* m_name;
        };

    private:
        const Conversion* m_conversions;
        mutable std::string m_string;

    public:
        EnumFilter( const Conversion* conversions );
        int to_memory( const std::string& value ) const;
        const std::string& to_archive( int value ) const;
};

SWEET_PERSIST_DECLSPEC extern const EnumFilter::Conversion BASIC_TYPES[];
SWEET_PERSIST_DECLSPEC EnumFilter enum_filter( const EnumFilter::Conversion* conversions );

}

}

#endif

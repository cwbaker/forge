//
// Ucs2CharFilter.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Ucs2CharFilter.hpp"
#include <sweet/assert/assert.hpp>

using namespace sweet::persist;

static const int UTF8_MAX_LENGTH = 6;

// The number of bytes in a UTF-8 character sequence indexed by the initial 
// byte in the sequence.
static const int UTF8_LENGTH[256] = 
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6
};

// A value that is subtracted from the character value that is calculated
// from the sum of the bytes in a UTF-8 sequence.  It adjusts for the length
// marker in the first byte (0x00, 0xc0, 0xe0, 0xf0, 0xf8, or 0xfc) and the 
// marker in each of the trailing bytes (0x80).
static const unsigned int UTF8_LENGTH_MASK[7] = 
{ 
    0x00000000ul,
    0x00000000ul, 
    0x00003080ul, 
    0x000e2080ul, 
    0x03c82080ul, 
    0xfa082080ul, 
    0x82082080ul 
};

void Ucs2CharFilter::to_archive( const wchar_t* src_begin, const wchar_t* src_end, char* dest_begin, char* dest_end ) const
{
    const wchar_t* src = src_begin;
    char* dest = dest_begin;
    while ( src < src_end && dest < dest_end )
    {
        if ( *src < 0x80 )
        {
            *(dest + 0) = static_cast<char>( *src );
            dest += 1;
        }
        else if ( *src < 0x800 )
        {
            SWEET_ASSERT( dest + 1 < dest_end );
            *(dest + 0) = static_cast<char>( 0xc0 | ((*src >> 6) & 0x1f) );
            *(dest + 1) = static_cast<char>( 0x80 | ((*src >> 0) & 0x3f) );
            dest += 2;            
        }
        else
        {
            SWEET_ASSERT( dest + 2 < dest_end );
            *(dest + 0) = static_cast<char>( 0xe0 | ((*src >> 12) & 0x0f) );
            *(dest + 1) = static_cast<char>( 0x80 | ((*src >>  6) & 0x3f) );
            *(dest + 2) = static_cast<char>( 0x80 | ((*src >>  0) & 0x3f) );
            dest += 3;            
        }

        ++src;
    }
}

void Ucs2CharFilter::to_memory( const char* src_begin, const char* src_end, wchar_t* dest_begin, wchar_t* dest_end ) const
{
    const char* src = src_begin;
    wchar_t* dest = dest_begin;
    while ( src < src_end && dest < dest_end )
    {
        wchar_t character = 0;

        int length = UTF8_LENGTH [*reinterpret_cast<const unsigned char*>(src)];
        if ( src + length > src_end )
        {
            break;
        }
        
        for ( int i = 0; i < length; ++i )
        {
            character <<= 6;
            character += *reinterpret_cast<const unsigned char*>( src );
            ++src;
        }        

        character -= UTF8_LENGTH_MASK [length];

        *dest = character;
        ++dest;
    }
}

unsigned int Ucs2CharFilter::to_archive_length( const wchar_t* begin, const wchar_t* end ) const
{
    const wchar_t* src = begin;
    unsigned int length = 0;
    while ( src < end )
    {
        if ( *src < 0x80 )
        {
            length += 1;
        }
        else if ( *src < 0x800 )
        {
            length += 2;            
        }
        else
        {
            length += 3;            
        }

        ++src;
    }

    return length;
}

unsigned int Ucs2CharFilter::to_memory_length( const char* begin, const char* end ) const
{
    unsigned int length = 0;
    const char* src = begin;
    while ( src < end )
    {
        ++length;
        src += UTF8_LENGTH [*reinterpret_cast<const unsigned char*>(src)];
    }
    return length;
}

Ucs2CharFilter sweet::persist::ucs2_char_filter()
{
    return Ucs2CharFilter();
}

//
// strings.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_STRINGS_IPP_INCLUDED
#define SWEET_PERSIST_STRINGS_IPP_INCLUDED

#include <string>

namespace sweet
{

namespace persist
{

template <class Archive, class CHAR, class TRAITS, class ALLOCATOR>
void save( Archive& archive, int mode, const char* name, std::basic_string<CHAR, TRAITS, ALLOCATOR>& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, 1 );
    archive.value( "value", value );
}

template <class Archive, class CHAR, class TRAITS, class ALLOCATOR>
void load( Archive& archive, int mode, const char* name, std::basic_string<CHAR, TRAITS, ALLOCATOR>& value )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    archive.value( "value", value );
}

template <class Archive, class CHAR, class TRAITS, class ALLOCATOR>
void resolve( Archive& archive, int mode, std::basic_string<CHAR, TRAITS, ALLOCATOR>& value )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );
}

}

}

#endif

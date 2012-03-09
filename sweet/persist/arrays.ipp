//
// arrays.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_ARRAYS_IPP_INCLUDED
#define SWEET_PERSIST_ARRAYS_IPP_INCLUDED

#include "ObjectGuard.hpp"
#include "types.hpp"
#include "objects.ipp"

namespace sweet
{

namespace persist
{

template <class Archive, class Type>
void save( Archive& archive, int mode, const char* name, const char* child_name, Type& values, size_t length )
{
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, length );
    for ( unsigned int i = 0; i < length; ++i )
    {
        save( archive, mode, child_name, values[i] );
    }
}

template <class Archive, class Type>
void load( Archive& archive, int mode, const char* name, const char* child_name, Type& values, size_t length )
{
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    if ( archive.is_object() )
    {
        unsigned int i = 0;
        while ( i < length && archive.find_next_object(child_name) )
        {
            load( archive, mode, child_name, values[i] );
            ++i;
        }
    }
}

template <class Archive, class Type>
void resolve( Archive& archive, int mode, Type& values, size_t length )
{
    ObjectGuard<Archive> guard( archive, 0, 0, MODE_VALUE );

    for ( unsigned int i = 0; i < length; ++i )
    {
        resolve( archive, mode, values[i] );
    }
}

template <class Archive, class Type, size_t LENGTH>
void save( Archive& archive, int mode, const char* name, Type (&values)[LENGTH] )
{
    save( archive, mode, name, "item", values, LENGTH );
}

template <class Archive, class Type, size_t LENGTH>
void load( Archive& archive, int mode, const char* name, Type (&values)[LENGTH] )
{
    load( archive, mode, name, "item", values, LENGTH );
}

template <class Archive, class Type, size_t LENGTH>
void resolve( Archive& archive, int mode, Type (&values)[LENGTH] )
{
    resolve( archive, mode, values, LENGTH );
}

}

}

#endif

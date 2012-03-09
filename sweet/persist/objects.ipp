//
// objects.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_OBJECTS_IPP_INCLUDED
#define SWEET_PERSIST_OBJECTS_IPP_INCLUDED

//#include "objects.hpp"
#include "ObjectGuard.hpp"
#include <sweet/assert/assert.hpp>

namespace sweet
{

namespace persist
{

template <class Type> struct creator
{
    static Type create()
    {
        return Type();
    }
};

template <class Type> struct resolver
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        SWEET_ASSERT( false );
    }
};

template <class Archive, class Type> void save( Archive& archive, int mode, const char* name, Type& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );    
    ObjectGuard<Archive> guard( archive, name, &object, mode, 1 );
    object.persist( archive );
}

template <class Archive, class Type> void load( Archive& archive, int mode, const char* name, Type& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    if ( archive.is_object() )
    {
        object.persist( archive );
    }
}

template <class Archive, class Type> void resolve( Archive& archive, int mode, Type& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    
    ObjectGuard<Archive> guard( archive, 0, &object, MODE_VALUE );    
    if ( archive.is_object() )
    {
        archive.track( &object, 0 );
        object.persist( archive );
    }
}

}

}

#endif

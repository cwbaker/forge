//
// intrusive_ptr.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_INTRUSIVE_PTR_HPP_INCLUDED
#define SWEET_PERSIST_INTRUSIVE_PTR_HPP_INCLUDED

#include "ObjectGuard.hpp"
#include <boost/intrusive_ptr.hpp>

namespace sweet
{

namespace persist
{

template <class Archive, class Type>
void save( Archive& archive, int mode, const char* name, boost::intrusive_ptr<Type>& object )
{
    ObjectGuard<Archive> guard( archive, name, object.get(), mode, object != 0 ? 1 : 0 );
    switch ( mode )
    {
        case MODE_VALUE:
            if ( object.get() )
            {
                archive.persist( SWEET_TYPEID(*object.get()), archive, object.get() );
            }
            break;

        case MODE_REFERENCE:
            break;

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Archive, class Type>
void load( Archive& archive, int mode, const char* name, boost::intrusive_ptr<Type>& object )
{
    SWEET_ASSERT( object == NULL );

    ObjectGuard<Archive> guard( archive, name, 0, mode );
    switch ( mode )
    {
        case MODE_VALUE:
            object = static_cast<Type*>( archive.create_and_persist<Type>() );
            break;

        case MODE_REFERENCE:
            archive.reference( archive.get_address(), reinterpret_cast<void**>(&object), &resolver<boost::intrusive_ptr<Type>>::resolve );
            break;

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Archive, class Type>
void resolve( Archive& archive, int mode, boost::intrusive_ptr<Type>& object )
{
    ObjectGuard<Archive> guard( archive, 0, object.get(), mode );
    switch ( mode )
    {
        case MODE_VALUE:
            if ( object.get() )
            {
                archive.track( object.get(), &object );
                archive.persist( SWEET_TYPEID(*object.get()), archive, object.get() );
            }
            break;

        case MODE_REFERENCE:
            archive.reference( 0, reinterpret_cast<void**>(&object), &resolver<boost::intrusive_ptr<Type>>::resolve );
            break;

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Type>
struct resolver<boost::intrusive_ptr<Type>>
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        boost::intrusive_ptr<Type>* referer = static_cast<boost::intrusive_ptr<Type>*>( reference );
        Type* object  = static_cast<Type*>( raw_ptr );
        *referer = object;
    }
};

}

}

#endif

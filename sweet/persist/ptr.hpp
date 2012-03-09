//
// ptr.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_PTR_HPP_INCLUDED
#define SWEET_PERSIST_PTR_HPP_INCLUDED

#include "ObjectGuard.hpp"
#include <sweet/pointer/ptr.hpp>

namespace sweet
{

namespace persist
{

template <class Archive, class Type>
void save( Archive& archive, int mode, const char* name, ptr<Type>& object )
{
    ObjectGuard<Archive> guard( archive, name, object.get(), mode, object.get() != 0 ? 1 : 0 );
    switch ( mode )
    {
        case MODE_VALUE:
            if ( object.get() != 0 )
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
void load( Archive& archive, int mode, const char* name, ptr<Type>& object )
{
    SWEET_ASSERT( object.get() == NULL );

    ObjectGuard<Archive> guard( archive, name, 0, mode );
    switch ( mode )
    {
        case MODE_VALUE:
            object.reset( static_cast<Type*>(archive.create_and_persist<Type>()) );
            break;

        case MODE_REFERENCE:
            archive.reference( archive.get_address(), reinterpret_cast<void**>(&object), &resolver<ptr<Type>>::resolve );
            break;

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Archive, class Type>
void resolve( Archive& archive, int mode, ptr<Type>& object )
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
            archive.reference( 0, reinterpret_cast<void**>(&object), &resolver<ptr<Type>>::resolve );
            break;

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Type>
struct resolver<ptr<Type>>
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        if ( smart_ptr )
        {
            ptr<Type>* referer = static_cast<ptr<Type>*>( reference );
            ptr<Type>* owner = static_cast<ptr<Type>*>( smart_ptr );
            *referer = *owner;
        }
    }
};

template <class Archive, class Type>
void save( Archive& archive, int mode, const char* name, weak_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_REFERENCE );

    ptr<Type> locked_object = object.lock();
    if ( locked_object )
    {
        ObjectGuard<Archive> guard( archive, name, locked_object.get(), MODE_REFERENCE, 1 );
    }
    else
    {
        ObjectGuard<Archive> guard( archive, name, NULL, MODE_REFERENCE, 0 );
    }
}

template <class Archive, class Type>
void load( Archive& archive, int mode, const char* name, weak_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_REFERENCE );
    SWEET_ASSERT( object.lock() == ptr<Type>() );

    ObjectGuard<Archive> guard( archive, name, 0, MODE_REFERENCE );
    archive.reference( archive.get_address(), reinterpret_cast<void**>(&object), &resolver<weak_ptr<Type>>::resolve );
}

template <class Archive, class Type>
void resolve( Archive& archive, int mode, weak_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_REFERENCE );
    archive.reference( 0, reinterpret_cast<void**>(&object), &resolver<weak_ptr<Type>>::resolve );
}

template <class Type>
struct resolver<weak_ptr<Type>>
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        weak_ptr<Type>* referer = static_cast<weak_ptr<Type>*>( reference );
        ptr<Type>* owner = static_cast<ptr<Type>*>( smart_ptr );
        *referer = *owner;
    }
};

}

}

#endif

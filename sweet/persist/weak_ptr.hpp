//
// weak_ptr.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_WEAK_PTR_HPP_INCLUDED
#define SWEET_PERSIST_WEAK_PTR_HPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Archive, class Type>
void save( Archive& archive, int mode, const char* name, boost::weak_ptr<Type>& object )
{
//
// A weak_ptr can only ever be a reference and so it is always written out
// as such.
//
    SWEET_ASSERT( mode == MODE_REFERENCE );
    if ( !object.expired() )
    {
        boost::shared_ptr<Type> locked_object = object.lock();
        ObjectGuard<Archive> guard( archive, name, locked_object.get(), MODE_REFERENCE, 1 );
    }
    else
    {
        ObjectGuard<Archive> guard( archive, name, 0, MODE_REFERENCE, 0 );
    }
}

template <class Archive, class Type>
void load( Archive& archive, int mode, const char* name, boost::weak_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_REFERENCE );
    SWEET_ASSERT( object.expired() );

    ObjectGuard<Archive> guard( archive, name, 0, MODE_REFERENCE );
    archive.reference( archive.get_address(), reinterpret_cast<void**>(&object), &resolver<boost::weak_ptr<Type>>::resolve );
}

template <class Archive, class Type>
void resolve( Archive& archive, int mode, boost::weak_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_REFERENCE );
    archive.reference( 0, reinterpret_cast<void**>(&object), &resolver<boost::weak_ptr<Type>>::resolve );
}

template <class Type>
struct resolver<boost::weak_ptr<Type>>
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        boost::weak_ptr<Type>* referer = static_cast<boost::weak_ptr<Type>*>( reference );
        boost::shared_ptr<Type>* owner = static_cast<boost::shared_ptr<Type>*>( smart_ptr );
        *referer = *owner;
    }
};

}

}

#endif

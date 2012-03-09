//
// scoped_ptr.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_SCOPED_PTR_HPP_INCLUDED
#define SWEET_PERSIST_SCOPED_PTR_HPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Archive, class Type>
void save( Archive& archive, int mode, const char* name, boost::scoped_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );

    ObjectGuard<Archive> guard( archive, name, object.get(), mode, object.get() != 0 ? 1: 0 );
    if ( object.get() )
    {
        archive.persist( SWEET_TYPEID(*object.get()), archive, object.get() );
    }
}

template <class Archive, class Type>
void load( Archive& archive, int mode, const char* name, boost::scoped_ptr<Type>& object )
{
    SWEET_ASSERT( object.get() == NULL );
    SWEET_ASSERT( mode == MODE_VALUE );

    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    object.reset( static_cast<Type*>(archive.create_and_persist<Type>()) );
}

template <class Archive, class Type>
void resolve( Archive& archive, int mode, boost::scoped_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );

    ObjectGuard<Archive> guard( archive, 0, object.get(), MODE_VALUE );
    if ( object.get() != NULL )
    {
        archive.track( object.get(), &object );
        archive.persist( SWEET_TYPEID(*object.get()), archive, object.get() );
    }
}

}

}

#endif

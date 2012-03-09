//
// auto_ptr.hpp
// Copyright (c) 2006  - 2010 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_AUTO_PTR_HPP_INCLUDED
#define SWEET_PERSIST_AUTO_PTR_HPP_INCLUDED

namespace sweet
{

namespace persist
{

/**
// Save a heap allocated object owned by an auto_ptr.
//
// The \e mode parameter must be MODE_VALUE because an auto_ptr always
// owns the object that it points to - it can never be a reference.
//
// @param archive
//  The Archive to save the object to.
//
// @param mode
//  Must be MODE_VALUE.
//
// @param name
//  The name of the element to save the object to.
//
// @param value
//  The auto_ptr that owns the object to save.
//
// @relates auto_ptr
*/
template <class Archive, class Type>
void
save( Archive& archive, int mode, const char* name, std::auto_ptr<Type>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );

    ObjectGuard<Archive> guard( archive, name, object.get(), mode, object.get() != 0 ? 1: 0 );
    if ( object.get() != NULL )
    {
        archive.persist( SWEET_TYPEID(*object.get()), archive, object.get() );
    }
}


/**
// Load a heap allocated object owned by an auto_ptr.
//
// The \e mode parameter must be MODE_VALUE because an auto_ptr always
// owns the object that it points to - it can never be a reference.
//
// @param archive
//  The Archive to load the object from.
//
// @param mode
//  Must be MODE_VALUE.
//
// @param name
//  The name of the element to load the object from.
//
// @param value
//  The auto_ptr to give ownership of the object to (must be empty when
//  passed to this function).
//
// @relates auto_ptr
*/
template <class Archive, class Type>
void
load( Archive& archive, int mode, const char* name, std::auto_ptr<Type>& object )
{
    SWEET_ASSERT( object.get() == NULL );
    SWEET_ASSERT( mode == MODE_VALUE );

    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    object.reset( static_cast<Type*>(archive.create_and_persist<Type>()) );
}


/**
// Resolve references made from a heap allocated object owned by 
// an auto_ptr.
//
// @param archive
//  The Archive to use to resolve references.
//
// @param mode
//  Must be MODE_VALUE.
//
// @param value
//  The auto_ptr that owns the object to resolve references in.
//
// @relates auto_ptr
*/
template <class Archive, class Type>
void
resolve( Archive& archive, int mode, std::auto_ptr<Type>& object )
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

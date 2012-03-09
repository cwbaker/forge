//
// functions.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_FUNCTIONS_IPP_INCLUDED
#define SWEET_PERSIST_FUNCTIONS_IPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Type> 
void* create()
{
    return static_cast<void*>( new Type() );
}

template <class Archive, class Type> 
void persist( Archive& archive, void* object )
{
    Type* type = static_cast<Type*>( object );
    SWEET_ASSERT( type );
    type->persist( archive );
}

}

}

#endif

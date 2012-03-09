//
// hash_set.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_HASH_SET_HPP_INCLUDED
#define SWEET_PERSIST_HASH_SET_HPP_INCLUDED

#include "sets.ipp"

namespace sweet
{

namespace persist
{

template <class Archive, class Type, class PREDICATE, class ALLOCATOR> 
void save( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_set<Type, PREDICATE, ALLOCATOR>& container )
{
    save_stl_set( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR> 
void load( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_set<Type, PREDICATE, ALLOCATOR>& container )
{
    SWEET_ASSERT( container.empty() );
    load_stl_set( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR> 
void resolve( Archive& archive, int mode, stdext::hash_set<Type, PREDICATE, ALLOCATOR>& container )
{
    resolve_stl_set( archive, mode, container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR>
void save( Archive& archive, int mode, const char* name, stdext::hash_set<Type, PREDICATE, ALLOCATOR>& container )
{
    save( archive, mode, name, "item", container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR>
void load( Archive& archive, int mode, const char* name, stdext::hash_set<Type, PREDICATE, ALLOCATOR>& container )
{
    load( archive, mode, name, "item", container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR> 
void save( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_multiset<Type, PREDICATE, ALLOCATOR>& container )
{
    save_stl_set( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR> 
void load( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_multiset<Type, PREDICATE, ALLOCATOR>& container )
{
    load_stl_multiset( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR> 
void resolve( Archive& archive, int mode, stdext::hash_multiset<Type, PREDICATE, ALLOCATOR>& container )
{
    resolve_stl_set( archive, mode, container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR>
void save( Archive& archive, int mode, const char* name, stdext::hash_multiset<Type, PREDICATE, ALLOCATOR>& container )
{
    save( archive, mode, name, "item", container );
}

template <class Archive, class Type, class PREDICATE, class ALLOCATOR>
void load( Archive& archive, int mode, const char* name, stdext::hash_multiset<Type, PREDICATE, ALLOCATOR>& container )
{
    load( archive, mode, name, "item", container );
}

}

}

#endif

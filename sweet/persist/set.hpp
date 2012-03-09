//
// set.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_SET_HPP_INCLUDED
#define SWEET_PERSIST_SET_HPP_INCLUDED

#include "sets.ipp"

namespace sweet
{

namespace persist
{

template <class Archive, class Type, class Predicate, class Allocator> 
void save( Archive& archive, int mode, const char* name, const char* child_name, std::set<Type, Predicate, Allocator>& container )
{
    save_stl_set( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class Predicate, class Allocator> 
void load( Archive& archive, int mode, const char* name, const char* child_name, std::set<Type, Predicate, Allocator>& container )
{
    load_stl_set( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class Predicate, class Allocator> 
void resolve( Archive& archive, int mode, std::set<Type, Predicate, Allocator>& container )
{
    resolve_stl_set( archive, mode, container );
}

template <class Archive, class Type, class Predicate, class Allocator>
void save( Archive& archive, int mode, const char* name, std::set<Type, Predicate, Allocator>& container )
{
    save( archive, mode, name, "item", container );
}

template <class Archive, class Type, class Predicate, class Allocator>
void load( Archive& archive, int mode, const char* name, std::set<Type, Predicate, Allocator>& container )
{
    load( archive, mode, name, "item", container );
}

template <class Archive, class Type, class Predicate, class Allocator> 
void save( Archive& archive, int mode, const char* name, const char* child_name, std::multiset<Type, Predicate, Allocator>& container )
{
    save_stl_set( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class Predicate, class Allocator> 
void load( Archive& archive, int mode, const char* name, const char* child_name, std::multiset<Type, Predicate, Allocator>& container )
{
    load_stl_multiset( archive, mode, name, child_name, container );
}

template <class Archive, class Type, class Predicate, class Allocator> 
void resolve( Archive& archive, int mode, std::multiset<Type, Predicate, Allocator>& container )
{
    resolve_stl_set( archive, mode, container );
}

template <class Archive, class Type, class Predicate, class Allocator>
void save( Archive& archive, int mode, const char* name, std::multiset<Type, Predicate, Allocator>& container )
{
    save( archive, mode, name, "item", container );
}

template <class Archive, class Type, class Predicate, class Allocator>
void load( Archive& archive, int mode, const char* name, std::multiset<Type, Predicate, Allocator>& container )
{
    load( archive, mode, name, "item", container );
}

}

}

#endif

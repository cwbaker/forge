//
// hash_map.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_HASH_MAP_HPP_INCLUDED
#define SWEET_PERSIST_HASH_MAP_HPP_INCLUDED

#include "utility.hpp"
#include "maps.ipp"

namespace sweet
{

namespace persist
{

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR> 
void save( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_map<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    save_stl_map( archive, mode, name, child_name, container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR> 
void load( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_map<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    load_stl_map( archive, mode, name, child_name, container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR> 
void resolve( Archive& archive, int mode, stdext::hash_map<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    resolve_stl_sequence( archive, mode, container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR>
void save( Archive& archive, int mode, const char* name, stdext::hash_map<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    save( archive, mode, name, "item", container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR>
void load( Archive& archive, int mode, const char* name, stdext::hash_map<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    load( archive, mode, name, "item", container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR> 
void save( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_multimap<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    save_stl_map( archive, mode, name, child_name, container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR> 
void load( Archive& archive, int mode, const char* name, const char* child_name, stdext::hash_multimap<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    load_stl_multimap( archive, mode, name, child_name, container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR> 
void resolve( Archive& archive, int mode, stdext::hash_multimap<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    resolve_stl_sequence( archive, mode, container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR>
void save( Archive& archive, int mode, const char* name, stdext::hash_multimap<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    save( archive, mode, name, "item", container );
}

template <class Archive, class KEY, class DATA, class PREDICATE, class ALLOCATOR>
void load( Archive& archive, int mode, const char* name, stdext::hash_multimap<KEY, DATA, PREDICATE, ALLOCATOR>& container )
{
    load( archive, mode, name, "item", container );
}

}

}

#endif

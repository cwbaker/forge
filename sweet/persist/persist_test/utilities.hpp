//
// utilities.hpp
// Copyright (c) 2006 - 2007 Charles Baker.
//

#ifndef UTILITIES_HPP_INCLUDED
#define UTILITIES_HPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class CONTAINER>
bool compare( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && *i == *j )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER, class INNER_CONTAINER>
bool 
compare_container_of_containers( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && compare(*i, *j) )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
bool compare_maps( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && i->first == j->first && i->second == j->second )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
bool compare_pointers( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && (*i == 0 && *j == 0 || *i != 0 && *j != 0 && *(*i) == *(*j)) )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
bool compare_pointer_types( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && (*i == 0 && *j == 0 || *i != 0 && *j != 0 && (*(*i) == *(*j) && SWEET_TYPEID(*(*i)) == SWEET_TYPEID(*(*j)))) )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
bool compare_weak_pointer_types( const CONTAINER& lhs, const CONTAINER& rhs )
{
    return true;
}


template <class CONTAINER>
bool compare_map_pointers( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && i->first == j->first && (i->second == 0 && j->second == 0 || i->second != 0 && j->second != 0 && *i->second == *j->second) )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
bool compare_weak_ptrs( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && (i->lock() == 0 && j->lock() == 0 || i->lock() != 0 && j->lock() != 0 && *(i->lock()) == *(j->lock())) )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
bool compare_map_weak_ptrs( const CONTAINER& lhs, const CONTAINER& rhs )
{
    typename CONTAINER::const_iterator i = lhs.begin();
    typename CONTAINER::const_iterator j = rhs.begin();
    while ( i != lhs.end() && j != rhs.end() && (i->second.lock().get() == 0 && j->second.lock().get() == 0 || i->second.lock() != 0 && j->second.lock() != 0 && *(i->second.lock()) == *(j->second.lock())) )
    {
        ++i;
        ++j;
    }

    return i == lhs.end() && j == rhs.end();
}


template <class CONTAINER>
void
clear_pointers( CONTAINER& container )
{
    typename CONTAINER::iterator i = container.begin();
    while ( i != container.end() )
    {
        delete *i;
        ++i;
    }

    container.clear();
}


template <class CONTAINER>
void
clear_map_pointers( CONTAINER& container )
{
    typename CONTAINER::iterator i = container.begin();
    while ( i != container.end() )
    {
        delete i->second;
        ++i;
    }

    container.clear();
}


template <class Type>
struct less_pointer
{
    bool operator()( const Type& item0, const Type& item1 ) const
    {
        return *item0 < *item1;
    }
};

template <class Type>
struct less_weak_ptr
{
    bool operator()( const boost::weak_ptr<Type> lhs, const boost::weak_ptr<Type> rhs ) const
    {
        boost::shared_ptr<Type> shared_lhs = lhs.lock();
        boost::shared_ptr<Type> shared_rhs = rhs.lock();   
        return *shared_lhs < *shared_rhs;
    }
};


template <class CONTAINER, class PREDICATE = std::less<typename CONTAINER::value_type>>
struct less_containers
{
    bool operator()( const CONTAINER& lhs, const CONTAINER& rhs )
    {
        std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), PREDICATE );
    }
};


/**
// A hash compare function template that dereferences it's key to generate the
// hash value and to compare two keys.  
//
// This is intended to be used to easily generate hash compare functors for 
// hash based associative containers storing raw or smart pointer types.
//
// The objects pointed to are expected to provide operator size_t() to 
// generate the hash value and operator<() for the comparison.
*/
template <class KEY>
class hash_compare_pointers : public stdext::hash_compare<KEY, less_pointer<KEY>>
{
    public:
        hash_compare_pointers() 
        : stdext::hash_compare<KEY, less_pointer<KEY>>()
        {
        };

        hash_compare_pointers( less_pointer<KEY> compare ) 
        : stdext::hash_compare<KEY, less_pointer<KEY>>( compare )
        {
        }

        size_t operator()( const KEY& key ) const
        {
            return static_cast<size_t>( *key );
        }

        bool operator()( const KEY& lhs, const KEY& rhs ) const
        {
            return *lhs < *rhs;
        }
};


/**
// A hash compare function template that dereferences it's key to generate the
// hash value and to compare two keys.  
//
// This is intended to be used to easily generate hash compare functors for 
// hash based associative containers storing raw or smart pointer types.
//
// The objects pointed to are expected to provide operator size_t() to 
// generate the hash value and operator<() for the comparison.
*/
template <class KEY>
class hash_compare_weak_ptr : public stdext::hash_compare<boost::weak_ptr<KEY>, less_weak_ptr<KEY>>
{
    public:
        hash_compare_weak_ptr() 
        : stdext::hash_compare<boost::weak_ptr<KEY>, less_weak_ptr<KEY>>()
        {
        }

        hash_compare_weak_ptr( less_weak_ptr<KEY> compare ) 
        : stdext::hash_compare<boost::weak_ptr<KEY>, less_weak_ptr<KEY>>( compare )
        {
        }

        size_t operator()( const KEY& key ) const
        {
            boost::shared_ptr<KEY::element_type> shared_key = key.lock();   
            return static_cast<size_t>( *shared_key );
        }

        bool operator()( const KEY& lhs, const KEY& rhs ) const
        {
            boost::shared_ptr<KEY::element_type> shared_lhs = lhs.lock();
            boost::shared_ptr<KEY::element_type> shared_rhs = rhs.lock();   
            return *shared_lhs < *shared_rhs;
        }
};

};

};

#endif  // #ifndef UTILITIES_HPP_INCLUDED

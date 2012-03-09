//
// remove_const.hpp
// Copyright 2009 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_REMOVE_CONST_HPP_INCLUDED
#define SWEET_PERSIST_REMOVE_CONST_HPP_INCLUDED

namespace sweet
{

namespace persist
{

/**
// Remove const from an arbitrary type.
*/
template <class Type>
struct remove_const
{
    typedef typename Type type;
};

template <class Type>
struct remove_const<const Type>
{
    typedef typename Type type;    
};

}

}

#endif
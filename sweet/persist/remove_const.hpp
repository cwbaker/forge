//
// remove_const.hpp
// Copyright 2009 - 2012 Charles Baker.  All rights reserved.
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
    typedef Type type;
};

template <class Type>
struct remove_const<const Type>
{
    typedef Type type;    
};

}

}

#endif

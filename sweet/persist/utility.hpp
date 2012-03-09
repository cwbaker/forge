//
// utility.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_UTILITY_HPP_INCLUDED
#define SWEET_PERSIST_UTILITY_HPP_INCLUDED

#include "remove_const.hpp"

namespace sweet
{

namespace persist
{

template <class Archive, class First, class Second>
void save( Archive& archive, int mode, const char* name, std::pair<First, Second>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, &object, MODE_VALUE, 1 );
    save( archive, MODE_VALUE, "first",  const_cast<typename remove_const<First>::type&>(object.first) );
    save( archive, MODE_VALUE, "second", const_cast<typename remove_const<Second>::type&>(object.second) );
}

template <class Archive, class First, class Second>
void load( Archive& archive, int mode, const char* name, std::pair<First, Second>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    load( archive, MODE_VALUE, "first",  const_cast<typename remove_const<First>::type&>(object.first) );
    load( archive, MODE_VALUE, "second", const_cast<typename remove_const<Second>::type&>(object.second) );
}

template <class Archive, class First, class Second>
void resolve( Archive& archive, int mode, std::pair<First, Second>& object )
{
    ObjectGuard<Archive> guard( archive, 0, &object, MODE_VALUE );
    resolve( archive, MODE_VALUE, const_cast<typename remove_const<First>::type&>(object.first)   );
    resolve( archive, mode,       const_cast<typename remove_const<Second>::type&>(object.second) );
}

}

}

#endif

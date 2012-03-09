//
// utility.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_UTILITY_HPP_INCLUDED
#define SWEET_PERSIST_UTILITY_HPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Archive, class FIRST, class SECOND>
void save( Archive& archive, int mode, const char* name, std::pair<FIRST, SECOND>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, &object, MODE_VALUE, 1 );
    save( archive, MODE_VALUE, "first",  const_cast<remove_const<FIRST>::type&>(object.first) );
    save( archive, MODE_VALUE, "second", const_cast<remove_const<SECOND>::type&>(object.second) );
}

template <class Archive, class FIRST, class SECOND>
void load( Archive& archive, int mode, const char* name, std::pair<FIRST, SECOND>& object )
{
    SWEET_ASSERT( mode == MODE_VALUE );
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE );
    load( archive, MODE_VALUE, "first",  const_cast<remove_const<FIRST>::type&>(object.first) );
    load( archive, MODE_VALUE, "second", const_cast<remove_const<SECOND>::type&>(object.second) );
}

template <class Archive, class FIRST, class SECOND>
void resolve( Archive& archive, int mode, std::pair<FIRST, SECOND>& object )
{
    ObjectGuard<Archive> guard( archive, 0, &object, MODE_VALUE );
    resolve( archive, MODE_VALUE, const_cast<remove_const<FIRST>::type&>(object.first)   );
    resolve( archive, mode,       const_cast<remove_const<SECOND>::type&>(object.second) );
}

}

}

#endif

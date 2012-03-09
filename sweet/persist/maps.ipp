//
// maps.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_MAPS_IPP_INCLUDED
#define SWEET_PERSIST_MAPS_IPP_INCLUDED

#include "ObjectGuard.hpp"
#include "remove_const.hpp"

namespace sweet
{

namespace persist
{

template <class Archive, class CONTAINER>
void save_stl_map( Archive& archive, int mode, const char* name, const char* child_name, CONTAINER& container )
{
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, container.size() );
    typename CONTAINER::iterator i = container.begin();
    while ( i != container.end() )
    {
        ObjectGuard<Archive> guard( archive, child_name, 0, MODE_VALUE, 1 );
        save( archive, MODE_VALUE, "first", const_cast<remove_const<typename CONTAINER::key_type>::type&>(i->first) );
        save( archive, mode, "second", const_cast<remove_const<typename CONTAINER::referent_type>::type&>(i->second) );
        ++i;
    }
}

template <class Archive, class CONTAINER>
void load_stl_map( Archive& archive, int mode, const char* name, const char* child_name, CONTAINER& container )
{
    SWEET_ASSERT( container.empty() );

    ObjectGuard<Archive> sequence_guard( archive, name, 0, MODE_VALUE );
    if ( archive.is_object() )
    {
        while ( archive.find_next_object(child_name) )
        {
            typename CONTAINER::iterator position = container.end();
            {
                ObjectGuard<Archive> guard( archive, child_name, 0, MODE_VALUE );

                if ( archive.is_object() )
                {
                    typename CONTAINER::key_type key( creator<typename CONTAINER::key_type>::create() );
                    load( archive, MODE_VALUE, "first",  key );
                    position = container.insert( std::make_pair(key, creator<typename CONTAINER::mapped_type>::create()) ).first;
                    load( archive, mode, "second", position->second );
                }
            }

            int backward = 0;
            while ( position != container.end() )
            {
                ++position;
                ++backward;
            }

            if ( backward > 1 )
            {
                archive.move_reference_addresses_backward( backward );
            }                
        }
    }
}

template <class Archive, class CONTAINER>
void load_stl_multimap( Archive& archive, int mode, const char* name, const char* child_name, CONTAINER& container )
{
    SWEET_ASSERT( container.empty() );    

    ObjectGuard<Archive> sequence_guard( archive, name, 0, MODE_VALUE );
    if ( archive.is_object() )
    {
        while ( archive.find_next_object(child_name) )
        {
            typename CONTAINER::iterator position = container.end();
            {
                ObjectGuard<Archive> guard( archive, child_name, 0, MODE_VALUE );

                if ( archive.is_object() )
                {
                    typename CONTAINER::key_type key( creator<typename CONTAINER::key_type>::create() );
                    load( archive, MODE_VALUE, "first",  key );
                    position = container.insert( std::make_pair(key, creator<typename CONTAINER::mapped_type>::create()) );
                    load( archive, mode, "second", position->second );
                }
            }

            int backward = 0;
            while ( position != container.end() )
            {
                ++position;
                ++backward;
            }

            if ( backward > 1 )
            {
                archive.move_reference_addresses_backward( backward );
            }                
        }
    }
}

}

}

#endif

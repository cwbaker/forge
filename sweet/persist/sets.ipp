//
// sets.ipp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_SETS_IPP_INCLUDED
#define SWEET_PERSIST_SETS_IPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Container, class Type>
struct set_resolver
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        Container& container = *static_cast<Container*>( reference );
        container.insert( *static_cast<typename Container::value_type*>(smart_ptr) );
    }
};

template <class Container, class Type>
struct set_resolver<Container, Type*>
{
    static void resolve( void* reference, void* raw_ptr, void* smart_ptr )
    {
        Container& container = *static_cast<Container*>( reference );
        container.insert( static_cast<Type*>(raw_ptr) );
    }
};

template <class Archive, class Container>
void save_stl_set( Archive& archive, int mode, const char* name, const char* child_name, Container& container )
{
    ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, container.size() );

    archive.flag( PERSIST_PRESERVE_EMPTY_ELEMENTS );
    typename Container::iterator i = container.begin();
    while ( i != container.end() )
    {
        typename Container::value_type& value = const_cast<typename Container::value_type&>(*i);
        save( archive, mode, child_name, value );
        ++i;
    }
}

template <class Archive, class Container>
void load_stl_set( Archive& archive, int mode, const char* name, const char* child_name, Container& container )
{
    SWEET_ASSERT( container.empty() );

    ObjectGuard<Archive> sequence_guard( archive, name, 0, MODE_VALUE );
    switch ( mode )
    {
        case MODE_VALUE:
            if ( archive.is_object() )
            {
                while ( archive.find_next_object(child_name) )
                {
                    typename Container::value_type value( creator<typename Container::value_type>::create() );
                    load( archive, MODE_VALUE, child_name, value );
                    typename Container::const_iterator position = container.insert( value ).first;

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
            break;

        case MODE_REFERENCE:
        {
            int count = 0;

            if ( archive.is_object() )
            {
                while ( archive.find_next_object(child_name) )
                {
                    ObjectGuard<Archive> guard( archive, child_name, 0, MODE_REFERENCE );

                    if ( archive.is_object() )
                    {
                        const void* address = archive.get_address();
                        archive.reference( address, static_cast<void*>(&container), &set_resolver<Container, typename Container::value_type>::resolve );
                        ++count;
                    }
                }
            }    

            break;
        }

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Archive, class Container>
void load_stl_multiset( Archive& archive, int mode, const char* name, const char* child_name, Container& container )
{
    SWEET_ASSERT( container.empty() );

    ObjectGuard<Archive> sequence_guard( archive, name, 0, MODE_VALUE );
    switch ( mode )
    {
        case MODE_VALUE:
            if ( archive.is_object() )
            {
                while ( archive.find_next_object(child_name) )
                {
                    typename Container::value_type value( creator<typename Container::value_type>::create() );
                    load( archive, MODE_VALUE, child_name, value );
                    typename Container::const_iterator position = container.insert( value );

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
            break;

        case MODE_REFERENCE:
        {
            int count = 0;

            if ( archive.is_object() )
            {
                while ( archive.find_next_object(child_name) )
                {
                    ObjectGuard<Archive> guard( archive, child_name, 0, MODE_REFERENCE );

                    if ( archive.is_object() )
                    {
                        const void* address = archive.get_address();
                        archive.reference( address, static_cast<void*>(&container), &set_resolver<Container, typename Container::value_type>::resolve );
                        ++count;
                    }
                }
            }    

            break;
        }

        default:
            SWEET_ASSERT( false );
            break;
    }
}

template <class Archive, class Container>
void resolve_stl_set( Archive& archive, int mode, Container& container )
{
    ObjectGuard<Archive> sequence_guard( archive, 0, 0, MODE_VALUE );
    switch ( mode )
    {
        case MODE_VALUE:
            for ( typename Container::iterator i = container.begin(); i != container.end(); ++i )
            {
                typename Container::value_type& value = const_cast<typename Container::value_type&>( *i );
                resolve( archive, mode, value );
            }
            break;

        case MODE_REFERENCE:
        {
            int count = archive.get_count();
            while ( count > 0 )
            {
                ObjectGuard<Archive> guard( archive, 0, 0, MODE_REFERENCE );
                archive.reference( 0, static_cast<void*>(&container), &set_resolver<Container, typename Container::value_type>::resolve );
                --count;
            }
            break;
        }

        default:
            SWEET_ASSERT( false );
            break;
    }
}

}

}

#endif

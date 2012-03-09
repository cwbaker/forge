//
// Resolver.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_RESOLVER_HPP_INCLUDED
#define SWEET_PERSIST_RESOLVER_HPP_INCLUDED

#include "declspec.hpp"
#include "Writer.hpp"
#include "Address.hpp"
#include "Reference.hpp"
#include "types.hpp"
#include <list>
#include <set>
#include <stack>
#include <ctime>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// Performs reference resolution by making a pass over an Archive that has 
// just been read in to memory.
//
// This class is derived from Writer<Resolver> rather than Reader<Resolver>
// so that the structure of the model that has been read in to memory (the
// number of and contents of containers and objects that are stored through
// pointers) can be exploited during the resolution pass.  If it was derived
// from Reader<Resolver> then the only way to retrieve all of the extra 
// information would be to reread the archive.
//
// The Resolver builds collections of unresolved references and identified 
// addresses as it goes.
//
// When a reference is found the identified addresses are searched for a 
// matching identifier.  If one is found then the reference is resolved
// straight away.  If no matching identified address is found then the 
// reference is added to the collection of unresolved references for later
// resolution.
//
// When an identified address is found the identified address is added to 
// the identified addresses collection and any unresolved references that have
// a matching identifier are resolved to the address and removed from the 
// collection of unresolved addresses.
*/
class SWEET_PERSIST_DECLSPEC Resolver : public Writer<Resolver>
{
    public:
        typedef ArchiveTypeResolver archive_type;

    private:
        struct Object
        {
            const void* m_address;
            std::vector<const void*> m_reference_addresses;
            std::list<Object> m_objects;

            Object( const void* address )
            : m_address( address ),
              m_reference_addresses(),
              m_objects()
            {
            }
        };

        struct State
        {
            Mode m_mode;
            bool m_sequence;
            const Object* m_object;
            std::vector<const void*>::const_iterator m_current_reference_address;
            std::list<Object>::const_iterator m_current_object;

            State( Mode mode, const Object* object )
            : m_mode( mode ),
              m_sequence( false ),
              m_object( object ),
              m_current_reference_address(),
              m_current_object()
            {
                if ( m_object )
                {
                    m_current_reference_address = m_object->m_reference_addresses.begin();
                    m_current_object = m_object->m_objects.begin();
                }
            }
        };

        Object m_object; ///< The root Object.
        std::stack<State> m_state; ///< The state.
        std::stack<Object*> m_current_object; ///< The current Object as the Object tree is being built up in the reading pass.
        std::set<Address> m_addresses; ///< The identified addresses.
        std::multiset<Reference> m_references; ///< The references.

    public:
        Resolver();
        Resolver( const Resolver& resolver );
 
        void track( void* raw_ptr, void* smart_ptr );
        void reference( const void* address, void* reference, void (*resolve)(void* reference, void* raw_ptr, void* smart_ptr) );

        Mode get_mode() const;
        int get_count();

        void begin_reference_addresses( const void* address );
        void end_reference_addresses();
        void move_reference_addresses_backward( int distance );
        void add_reference_address( const void* address );

        void begin_object( const char* name, const void* address, Mode mode, int size );
        void end_object();
        bool is_object() const;
        void type( const std::string& type );
        void flag( int value );

        template <class Type> void process( int version, Type& object );
        template <class Type> void process( int version, const char* child_name, Type& container );
        template <class Type> void process( int version, const char* child_name, Type& values, size_t length );

        void value( const char* name, bool& value );
        void value( const char* name, char& value );
        void value( const char* name, signed char& value );
        void value( const char* name, unsigned char& value );
        void value( const char* name, wchar_t& value );
        void value( const char* name, short& value );
        void value( const char* name, unsigned short& value );
        void value( const char* name, int& value );
        void value( const char* name, unsigned int& value );
        void value( const char* name, long& value );
        void value( const char* name, unsigned long& value );
#if defined(BUILD_PLATFORM_MSVC)
        void value( const char* name, std::time_t& value );
#endif
        void value( const char* name, float& value );
        void value( const char* name, double& value );
        void value( const char* name, wchar_t* value, size_t max );
        void value( const char* name, std::wstring& value );
        void value( const char* name, char* value, size_t max );
        void value( const char* name, std::string& value );

        template <class Filter> void value( const char* name, wchar_t* value, size_t max, const Filter& filter );
        template <class Filter> void value( const char* name, std::wstring& value, const Filter& filter );
        template <class Filter> void value( const char* name, char* value, size_t max, const Filter& filter );
        template <class Filter> void value( const char* name, std::string& value, const Filter& filter );

        template <class Type, class Filter> void value( const char* name, Type& value, const Filter& filter );

        template <class Type> void value( const char* name, Type& object );
        template <class Type> void refer( const char* name, Type& object );

        template <class Type> void value( const char* name, const char* child_name, Type& container );
        template <class Type> void refer( const char* name, const char* child_name, Type& container );

        template <class Type, size_t LENGTH> void value( const char* name, const char* child_name, Type (& values)[LENGTH] );
        template <class Type, size_t LENGTH> void refer( const char* name, const char* child_name, Type (& values)[LENGTH] );
};

}

}

#endif

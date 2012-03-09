//
// TextReader.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_TEXTREADER_HPP_INCLUDED
#define SWEET_PERSIST_TEXTREADER_HPP_INCLUDED

#include "declspec.hpp"
#include "Element.hpp"
#include "Reader.hpp"
#include "Resolver.hpp"
#include "types.hpp"
#include <string>
#include <stack>
#include <ctime>

namespace sweet
{

namespace persist
{

/**
// Base class for text archive readers.
//
// Uses a parser to parse an archive from external storage into a tree of
// elements and attributes which is then persisted into an in memory model.
*/
class SWEET_PERSIST_DECLSPEC TextReader : public Reader<TextReader>
{
    struct State
    {
        typedef std::stack<State> stack;

        Element* m_element;
        Mode m_mode;
        bool m_sequence;
        std::list<Element>::iterator m_position;

        State( Element* element, Mode mode )
        : m_element( element ),
          m_mode( mode ),
          m_sequence( false ),
          m_position()
        {            
        }
    };

    Element m_element; ///< The root Element of the tree of Elements and Attributes.
    std::stack<State> m_state; ///< The state.
    Resolver m_resolver; ///< Resolves all references once the archive has been read in to memory.

    public:
        TextReader();
        TextReader( const TextReader& reader );

        void track( void* raw_ptr, void* smart_ptr );
        void reference( const void* address, void* reference, void (*resolve)(void* reference, void* raw_ptr, void* smart_ptr) );
        void move_reference_addresses_backward( int n );

        const std::string& get_type() const;
        const void* get_address() const;
 
        Mode get_mode() const;
        Element* get_element();
        Element* get_current_element();
        bool get_sequence() const;
        int get_count() const;

        void begin_object( const char* name, const void* address, Mode mode, int size );
        void end_object();
        bool find_next_object( const char* name );
        bool is_object() const;
        bool is_object_empty() const;
        bool is_reference() const;

        template <class Type> void read( const char* name, Type& object );
        template <class Type> void read( const char* name, const char* child_name, Type& container );
        template <class Type, size_t LENGTH> void read( const char* name, const char* child_name, Type (& values)[LENGTH] );

        template <class Type> void enter( const char* format, int version, Type& object );
        template <class Type> void declare( const char* name, int flags );

        void flag( int value );

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
        void value( const char* name, char* value, size_t max );
        void value( const char* name, wchar_t* value, size_t max );
        void value( const char* name, std::string& value );
        void value( const char* name, std::wstring& value );

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

    private:
        Element* find_element( const std::string& name );
};

}

}

#endif

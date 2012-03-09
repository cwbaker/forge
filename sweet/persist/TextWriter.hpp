//
// TextWriter.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_TEXTWRITER_HPP_INCLUDED
#define SWEET_PERSIST_TEXTWRITER_HPP_INCLUDED

#include "declspec.hpp"
#include "Writer.hpp"
#include "Element.hpp"
#include "types.hpp"
#include <stack>
#include <ctime>

namespace sweet
{

namespace persist
{

class Element;

/**
// Base class for text archive writers.
//
// Persists from an in memory model to an in memory tree of elements and
// attributes which is written out to storage using a writer class.
*/
class SWEET_PERSIST_DECLSPEC TextWriter : public Writer<TextWriter>
{
    struct State
    {
        Element* m_element;
        Mode m_mode;
        const void* m_address;

        State( Element* element, Mode mode, const void* address )
        : m_element( element ),
          m_mode( mode ),
          m_address( address )
        {
        }        
    };

    Element m_element; ///< The root Element in the tree that is being written to.
    std::stack<State> m_state; ///< The State stack.

    public:
        TextWriter();
        TextWriter( const TextWriter& writer );

        Mode get_mode() const;
        const Element* get_element() const;
        Element* get_current_element();

        void begin_object( const char* name, const void* address, Mode mode, int size );
        void end_object();

        void type( const std::string& type );

        template <class Type> void write( const char* name, Type& object );
        template <class Type> void write( const char* name, const char* child_name, Type& container );
        template <class Type, size_t LENGTH> void write( const char* name, const char* child_name, Type (& values)[LENGTH] );

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

        template <class Filter> void value( const char* name, char* value, size_t max, const Filter& filter );
        template <class Filter> void value( const char* name, wchar_t* value, size_t max, const Filter& filter );
        template <class Filter> void value( const char* name, std::string& value, const Filter& filter );
        template <class Filter> void value( const char* name, std::wstring& value, const Filter& filter );

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

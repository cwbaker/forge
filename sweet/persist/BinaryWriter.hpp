//
// BinaryWriter.hpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_BINARYWRITER_HPP_INCLUDED
#define SWEET_PERSIST_BINARYWRITER_HPP_INCLUDED

#include "declspec.hpp"
#include "Writer.hpp"
#include "types.hpp"
#include <stack>
#include <ctime>

namespace sweet
{

namespace persist
{

/**
// Writes binary archives.
*/
class SWEET_PERSIST_DECLSPEC BinaryWriter : public Writer<BinaryWriter>
{
    struct State
    {
        Mode m_mode;

        State( Mode mode )
        : m_mode( mode )
        {
        }
    };

    std::stack<State> m_state; ///< The state stack.
    std::ostream* m_ostream; ///< The output stream being written to.

    public:
        BinaryWriter();
        BinaryWriter( const BinaryWriter& writer );

        Mode get_mode() const;
        void begin_object( const char* name, const void* address, Mode mode, int size );
        void end_object();
        void type( const std::string& type );

        template <class Char, class Type> void write( const Char* filename, const Char* name, Type& object );
        template <class Char, class Type> void write( const Char* filename, const Char* name, const Char* child_name, Type& container );
        template <class Char, class Type, size_t LENGTH> void write( const Char* filename, const Char* name, const Char* child_name, Type (& values)[LENGTH] );

        template <class Char, class Type> void write( std::ostream& ostream, const Char* name, Type& object );
        template <class Char, class Type> void write( std::ostream& ostream, const Char* name, const Char* child_name, Type& container );
        template <class Char, class Type, size_t LENGTH> void write( std::ostream& ostream, const Char* name, const Char* child_name, Type (& values)[LENGTH] );

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

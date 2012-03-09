//
// BinaryReader.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_BINARYREADER_HPP_INCLUDED
#define SWEET_PERSIST_BINARYREADER_HPP_INCLUDED

#include "declspec.hpp"
#include "Reader.hpp"
#include "Resolver.hpp"
#include "types.hpp"

namespace sweet
{

namespace persist
{

/**
// Reads binary archives.
*/
class SWEET_PERSIST_DECLSPEC BinaryReader : public Reader<BinaryReader>
{
    struct State
    {
        Mode m_mode;
        int m_size;
        const void* m_address;
        bool m_sequence;

        State( Mode mode, int size, const void* address, bool sequence )
        : m_mode( mode ),
          m_size( size ),
          m_address( address ),
          m_sequence( sequence )
        {
        }
    };

    std::stack<State> m_state; ///< The stack of States.
    std::istream* m_istream; ///< The input stream being read from.
    std::string m_type; ///< The string that uniquely identifies the type of the object being read in (if any).
    Resolver m_resolver; ///< The Resolver that is used to resolve references after an archive has been read.
 
    public:
        BinaryReader();
        BinaryReader( const BinaryReader& reader );

        void track( void* raw_ptr, void* smart_ptr );
        void reference( const void* address, void* reference, void (*resolve)(void* reference, void* raw_ptr, void* smart_ptr) );
        void add_set_reference_count( int count );
        void move_reference_addresses_backward( int n );

        const std::string& get_type();
        const void* get_address();
        Mode get_mode() const;
        bool get_sequence() const;
        int get_count() const;

        void begin_object( const char* name, const void* address, Mode mode, int size );
        void end_object();
        bool find_next_object( const char* name );
        bool is_object() const;
        bool is_object_empty() const;
        bool is_reference() const;

        void type();

        template <class Char, class Type> void read( const Char* filename, const char* name, Type& object );
        template <class Char, class Type> void read( const Char* filename, const char* name, const char* child_name, Type& container );
        template <class Char, class Type, size_t LENGTH> void read( const Char* filename, const char* name, const char* child_name, Type (& values)[LENGTH] );

        template <class Type> void read( std::istream& istream, const char* name, Type& object );
        template <class Type> void read( std::istream& istream, const char* name, const char* child_name, Type& container );
        template <class Type, size_t LENGTH> void read( std::istream& istream, const char* name, const char* child_name, Type (& values)[LENGTH] );

        template <class Type> void enter( const char* format, int version, Type& object );
        template <class Type> void declare( const char* name, int flags );

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

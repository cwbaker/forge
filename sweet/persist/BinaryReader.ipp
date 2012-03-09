//
// BinaryReader.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_BINARYREADER_IPP_INCLUDED
#define SWEET_PERSIST_BINARYREADER_IPP_INCLUDED

#include "BinaryReader.hpp"
#include <fstream>
#include <string.h>

namespace sweet
{

namespace persist
{

template <class Char, class Type> 
void BinaryReader::read( const Char* filename, const char* name, Type& object )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    std::ifstream stream( filename, std::ios::binary );
    read( stream, name, object );
}

template <class Char, class Type> 
void BinaryReader::read( const Char* filename, const char* name, const char* child_name, Type& container )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    std::ifstream stream( filename, std::ios::binary );
    read( stream, name, child_name, container );
}

template <class Char, class Type, size_t LENGTH> 
void BinaryReader::read( const Char* filename, const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    std::ifstream stream( filename, std::ios::binary );
    read( stream, name, child_name, values );
}

template <class Type>
void BinaryReader::read( std::istream& istream, const char* name, Type& object )
{
    m_resolver.set_filename( get_filename() );
    m_resolver.set_contexts( get_contexts() );

    reset();
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE, 0, 0, false) );
    m_istream = &istream;
    istream.exceptions( std::istream::eofbit | std::istream::badbit | std::istream::failbit );

    load( *this, MODE_VALUE, 0, object );
    
    m_istream = NULL;
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );
    m_resolver.process( version(), object );
}

template <class Type>
void BinaryReader::read( std::istream& istream, const char* name, const char* child_name, Type& container )
{
    m_resolver.set_filename( get_filename() );
    m_resolver.set_contexts( get_contexts() );

    reset();
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE, 0, 0, false) );
    m_istream = &istream;
    istream.exceptions( std::istream::eofbit | std::istream::badbit | std::istream::failbit );

    load( *this, MODE_VALUE, 0, 0, container );

    m_istream = NULL;
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );
    m_resolver.process( version(), container );
}

template <class Type, size_t LENGTH>
void BinaryReader::read( std::istream& istream, const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    m_resolver.set_filename( get_filename() );
    m_resolver.set_contexts( get_contexts() );

    reset();
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE, 0, 0, false) );
    m_istream = &istream;
    istream.exceptions( std::istream::eofbit | std::istream::badbit | std::istream::failbit );

    load( *this, MODE_VALUE, 0, 0, values, LENGTH );

    m_istream = NULL;
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );
    m_resolver.process( version(), values, LENGTH );
}

template <class Type> 
void BinaryReader::enter( const char* format, int version, Type& object )
{
    Reader::enter( format, version, object );
    m_resolver.enter( format, version, object );
}

template <class Type> 
void BinaryReader::declare( const char* name, int flags )
{
    Reader::declare<Type>( name, flags );
    m_resolver.declare<Type>( name, flags );
}

template <class Filter> 
void BinaryReader::value( const char* name, wchar_t* value, size_t max, const Filter& filter )
{
    std::wstring archive_value;
    BinaryReader::value( NULL, archive_value );    
    std::wstring filtered_value = filter.to_memory( archive_value );
    wcsncpy( value, filtered_value.c_str(), max );
    value[std::min(max - 1, filtered_value.length())] = L'\0';
}

template <class Filter> 
void BinaryReader::value( const char* name, std::wstring& value, const Filter& filter )
{
    std::wstring archive_value;
    BinaryReader::value( NULL, archive_value );    
    value = filter.to_memory( archive_value );
}

template <class Filter> 
void BinaryReader::value( const char* name, char* value, size_t max, const Filter& filter )
{
    std::string archive_value;
    BinaryReader::value( NULL, archive_value );    
    std::string filtered_value = filter.to_memory( archive_value );
    strncpy( value, filtered_value.c_str(), max );
    value[std::min(max - 1, filtered_value.length())] = '\0';
}

template <class Filter> 
void BinaryReader::value( const char* name, std::string& value, const Filter& filter )
{
    std::string archive_value;
    BinaryReader::value( NULL, archive_value );    
    value = filter.to_memory( archive_value );
}

template <class Type, class Filter> 
void BinaryReader::value( const char* name, Type& value, const Filter& filter )
{
    SWEET_ASSERT( m_istream );
    m_istream->read( reinterpret_cast<char*>(&value), sizeof(value) );
}

template <class Type> 
void BinaryReader::value( const char* name, Type& object )
{
    load( *this, MODE_VALUE, NULL, object );
}

template <class Type> 
void BinaryReader::refer( const char* name, Type& object )
{
    load( *this, MODE_REFERENCE, NULL, object );
}

template <class Type> 
void BinaryReader::value( const char* name, const char* child_name, Type& container )
{
    load( *this, MODE_VALUE, 0, 0, container );
}

template <class Type> 
void BinaryReader::refer( const char* name, const char* child_name, Type& container )
{
    load( *this, MODE_REFERENCE, 0, 0, container );
}

template <class Type, size_t LENGTH> 
void BinaryReader::value( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    load( *this, MODE_VALUE, 0, 0, values, LENGTH );
}

template <class Type, size_t LENGTH> 
void BinaryReader::refer( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    load( *this, MODE_REFERENCE, 0, 0, values, LENGTH );
}

}

}

#endif

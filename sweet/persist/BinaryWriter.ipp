//
// BinaryWriter.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_BINARYWRITER_IPP_INCLUDED
#define SWEET_PERSIST_BINARYWRITER_IPP_INCLUDED

#include <fstream>

namespace sweet
{

namespace persist
{

template <class Char, class Type> 
void BinaryWriter::write( const Char* filename, const Char* name, Type& object )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    write( std::ofstream(filename, std::ios::binary), name, object );
}

template <class Char, class Type> 
void BinaryWriter::write( const Char* filename, const Char* name, const Char* child_name, Type& container )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    write( std::ofstream(filename, std::ios::binary), name, child_name, container );
}

template <class Char, class Type, size_t LENGTH> 
void BinaryWriter::write( const Char* filename, const Char* name, const Char* child_name, Type (& values)[LENGTH] )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    write( std::ofstream(filename, std::ios::binary), name, child_name, values );
}

template <class Char, class Type> 
void BinaryWriter::write( std::ostream& ostream, const Char* name, Type& object )
{
    reset();
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE) );
    m_ostream = &ostream;
    ostream.exceptions( std::ostream::eofbit | std::ostream::badbit | std::ostream::failbit );

    save( *this, MODE_VALUE, 0, object );
    object.exit( *this );

    m_ostream = NULL;
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );
}

template <class Char, class Type> 
void BinaryWriter::write( std::ostream& ostream, const Char* name, const Char* child_name, Type& container )
{
    reset();
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE) );
    m_ostream = &ostream;
    ostream.exceptions( std::ostream::eofbit | std::ostream::badbit | std::ostream::failbit );

    save( *this, MODE_VALUE, 0, 0, container );
    object.exit( *this );

    m_ostream = NULL;
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );
}

template <class Char, class Type, size_t LENGTH> 
void BinaryWriter::write( std::ostream& ostream, const Char* name, const Char* child_name, Type (& values)[LENGTH] )
{
    reset();
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE) );
    m_ostream = &ostream;
    ostream.exceptions( std::ostream::eofbit | std::ostream::badbit | std::ostream::failbit );

    save( *this, MODE_VALUE, 0, 0, values, LENGTH );
    object.exit( *this );

    m_ostream = NULL;
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );
}

template <class Filter> 
void BinaryWriter::value( const char* name, wchar_t* value, size_t max, Filter& filter )
{
    BinaryWriter::value( NULL, filter.to_archive(std::wstring(value, strnlen(value, max))) );
}

template <class Filter> 
void BinaryWriter::value( const char* name, std::wstring& value, Filter& filter )
{
    BinaryWriter::value( NULL, filter.to_archive(value) );
}

template <class Filter> 
void BinaryWriter::value( const char* name, char* value, size_t max, Filter& filter )
{
    BinaryWriter::value( NULL, filter.to_archive(std::string(value, strnlen(value, max))) );
}

template <class Filter> 
void BinaryWriter::value( const char* name, std::string& value, Filter& filter )
{
    BinaryWriter::value( NULL, filter.to_archive(value) );
}

template <class Type, class Filter>
void BinaryWriter::value( const char* name, Type& value, Filter& filter )
{
    SWEET_ASSERT( m_ostream );

    if ( get_mode() != MODE_REFERENCE )
    {
        m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
    }
}

template <class Type> 
void BinaryWriter::value( const char* name, Type& object )
{
    if ( get_mode() != MODE_REFERENCE )
    {
        save( *this, MODE_VALUE, 0, object );
    }
}

template <class Type> 
void BinaryWriter::refer( const char* name, Type& object )
{
    if ( get_mode() != MODE_REFERENCE )
    {
        save( *this, MODE_REFERENCE, 0, object );
    }
}

template <class Type> 
void BinaryWriter::value( const char* name, const char* child_name, Type& container )
{
    if ( get_mode() != MODE_REFERENCE )
    {
        save( *this, MODE_VALUE, 0, 0, container );
    }
}

template <class Type> 
void BinaryWriter::refer( const char* name, const char* child_name, Type& container )
{
    if ( get_mode() != MODE_REFERENCE )
    {
        save( *this, MODE_REFERENCE, 0, 0, container );
    }
}

template <class Type, size_t LENGTH> 
void BinaryWriter::value( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    if ( get_mode() != MODE_REFERENCE )
    {
        save( *this, MODE_VALUE, 0, 0, values, LENGTH );
    }
}

template <class Type, size_t LENGTH> 
void BinaryWriter::refer( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    if ( get_mode() != MODE_REFERENCE )
    {
        save( *this, MODE_REFERENCE, 0, 0, values, LENGTH );
    }
}

}

}

#endif

//
// BinaryWriter.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "BinaryWriter.hpp"
#include "Writer.ipp"
#include <string.h>
#include <wchar.h>

using namespace sweet::persist;

BinaryWriter::BinaryWriter()
: Writer<BinaryWriter>(),
  m_state(),
  m_ostream( NULL )
{
}

BinaryWriter::BinaryWriter( const BinaryWriter& writer )
: Writer<BinaryWriter>( writer ),
  m_state(),
  m_ostream( NULL )
{
}

Mode BinaryWriter::get_mode() const
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_mode;
}

void BinaryWriter::begin_object( const char* name, const void* address, Mode mode, int size )
{
    SWEET_ASSERT( m_ostream );
    SWEET_ASSERT( mode == MODE_VALUE || mode == MODE_REFERENCE );

    m_state.push( State(mode) );
    m_ostream->write( reinterpret_cast<const char*>(&size), sizeof(size) );
    m_ostream->write( reinterpret_cast<const char*>(&address), sizeof(address) );
}

void BinaryWriter::end_object()
{
    m_state.pop();
    SWEET_ASSERT( !m_state.empty() );
}

void BinaryWriter::type( const std::string& type )
{
    SWEET_ASSERT( !type.empty() );
    value( static_cast<const char*>(NULL), const_cast<std::string&>(type) );
}

void BinaryWriter::flag( int value )
{
}

void BinaryWriter::value( const char* name, bool& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, char& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, signed char& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, unsigned char& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, wchar_t& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, short& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, unsigned short& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, int& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, unsigned int& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, long& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, unsigned long& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

#if defined(BUILD_PLATFORM_MSVC)
void BinaryWriter::value( const char* name, time_t& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}
#endif

void BinaryWriter::value( const char* name, float& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, double& value )
{
    SWEET_ASSERT( m_ostream );
    m_ostream->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void BinaryWriter::value( const char* name, wchar_t* value, size_t max )
{
    SWEET_ASSERT( m_ostream );
    size_t length = wcslen( value );
    m_ostream->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    m_ostream->write( reinterpret_cast<const char*>(value), length * sizeof(wchar_t) );
}

void BinaryWriter::value( const char* name, std::wstring& value )
{
    SWEET_ASSERT( m_ostream );
    size_t length = value.length();
    m_ostream->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    m_ostream->write( reinterpret_cast<const char*>(value.c_str()), length * sizeof(wchar_t) );
}

void BinaryWriter::value( const char* name, char* value, size_t max )
{
    SWEET_ASSERT( m_ostream );
    size_t length = strlen( value );
    m_ostream->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    m_ostream->write( reinterpret_cast<const char*>(value), length );
}

void BinaryWriter::value( const char* name, std::string& value )
{
    SWEET_ASSERT( m_ostream );
    size_t length = value.length();
    m_ostream->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    m_ostream->write( reinterpret_cast<const char*>(value.c_str()), length );
}

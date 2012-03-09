//
// TextReader.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "TextReader.hpp"
#include "Resolver.hpp"
#include "Ucs2CharFilter.hpp"
#include "Reader.ipp"
#include "Writer.ipp"
#include <string.h>

using namespace sweet::persist;

TextReader::TextReader()
: Reader<TextReader>(),
  m_element(),
  m_state(),
  m_resolver()
{
}

TextReader::TextReader( const TextReader& reader )
: Reader<TextReader>( reader ),
  m_element(),
  m_state(),
  m_resolver( reader.m_resolver )
{
}

void TextReader::track( void* raw_ptr, void* smart_ptr )
{
}

void TextReader::reference( const void* address, void* reference, void (*resolve)(void* reference, void* raw_ptr, void* smart_ptr) )
{
    m_resolver.add_reference_address( address );
}

void TextReader::move_reference_addresses_backward( int n )
{
    m_resolver.move_reference_addresses_backward( n );
}

const std::string& TextReader::get_type() const
{
    SWEET_ASSERT( !m_state.empty() && m_state.top().m_element );

    Attribute* attribute = m_state.top().m_element->find_attribute( get_class_keyword() );
    if ( attribute == NULL )
    {
        static const std::string null_type( "" );
        return null_type;
    }
    return attribute->string();
}

const void* TextReader::get_address() const
{
    SWEET_ASSERT( !m_state.empty() );

    const void* address = NULL;
    if ( is_object() )
    {
        Attribute* attribute = m_state.top().m_element->find_attribute( get_address_keyword() );
        if ( attribute )
        {
            address = attribute->address();
        }
    }
    return address;
}

Mode TextReader::get_mode() const
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_mode;
}

Element* TextReader::get_element()
{
    return &m_element;
}

Element* TextReader::get_current_element()
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_element;
}

bool TextReader::get_sequence() const
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_sequence;
}

int TextReader::get_count() const
{
//
// This function is very different from the BinaryReader::get_count() 
// function in that it will always return the total
// number of objects that are in the sequence while BinaryReader::get_count()
// will return the number of items that are remaining to be read.  It is 
// assumed that this function is only called to reserve the number of elements
// to be read in for a std::vector before any of the objects have been read
// in both cases this will be the total number of objects to read in the
// sequence.
//
// Make sure that this function is not called after objects have been read
// from the sequence because BinaryReader::get_count() behaves quite 
// differently in that case (see the comment above).
//
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_element != NULL ? m_state.top().m_element->elements().size() : 0;
}

void TextReader::begin_object( const char* name, const void* address, Mode mode, int size )
{
    SWEET_ASSERT( !m_state.empty() );
    
    m_state.push( State(find_element(name), mode) );
    if ( mode == MODE_VALUE )
    {
        const void* address = NULL;
        if ( is_object() )
        {
            Attribute* attribute = get_current_element()->find_attribute( get_address_keyword() );
            if ( attribute != NULL )
            {
                address = attribute->address();
            }
        }
        m_resolver.begin_reference_addresses( address );
    }
}

void TextReader::end_object()
{
    if ( get_mode() == MODE_VALUE )
    {
        m_resolver.end_reference_addresses();
    }
    m_state.pop();
    SWEET_ASSERT( !m_state.empty() );
}

bool TextReader::find_next_object( const char* name )
{
    SWEET_ASSERT( !m_state.empty() );
    
    Element* element = m_state.top().m_element;
    SWEET_ASSERT( element );

    if ( !get_sequence() )
    {
        std::list<Element>::iterator i = element->elements().begin();
        while ( i != element->elements().end() && i->name() != name )
        {
            ++i;
        }

        m_state.top().m_position = i;
        m_state.top().m_sequence = true;
    }
    else if ( m_state.top().m_position != element->elements().end() )
    {
        ++m_state.top().m_position;

        std::list<Element>::iterator i = m_state.top().m_position;
        while ( i != element->elements().end() && i->name() != name )
        {
            ++i;
        }

        m_state.top().m_position = i;
    }

    return m_state.top().m_position != element->elements().end();
}

bool TextReader::is_object() const
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_element != 0;
}

bool TextReader::is_object_empty() const
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( m_state.top().m_element );
    return m_state.top().m_element->attributes().empty();
}

bool TextReader::is_reference() const
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( m_state.top().m_element );

    const Element* element = m_state.top().m_element;
    return element->elements().empty() && element->attributes().size() == 1 && element->attributes().front().name() == get_address_keyword();
}

void TextReader::flag( int value )
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( m_state.top().m_element );
    m_state.top().m_element->set_flag( value );
}

void TextReader::value( const char* name, bool& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = attribute->boolean();
    }
}

void TextReader::value( const char* name, char& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<char>( attribute->integer() );
    }
}

void TextReader::value( const char* name, signed char& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<signed char>( attribute->integer() );
    }
}

void TextReader::value( const char* name, unsigned char& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<unsigned char>( attribute->unsigned_integer() );
    }
}

void TextReader::value( const char* name, wchar_t& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<wchar_t>( attribute->unsigned_integer() );
    }
}

void TextReader::value( const char* name, short& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<short>( attribute->integer() );
    }
}

void TextReader::value( const char* name, unsigned short& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<unsigned short>( attribute->unsigned_integer() );
    }
}

void TextReader::value( const char* name, int& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = attribute->integer();
    }
}

void TextReader::value( const char* name, unsigned int& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = attribute->unsigned_integer();
    }
}

void TextReader::value( const char* name, long& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<long>( attribute->integer() );
    }
}

void TextReader::value( const char* name, unsigned long& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<unsigned long>( attribute->unsigned_integer() );
    }
}

#if defined(BUILD_PLATFORM_MSVC)
void TextReader::value( const char* name, std::time_t& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<std::time_t>( attribute->unsigned_integer() );
    }
}
#endif

void TextReader::value( const char* name, float& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<float>( attribute->real() );
    }
}

void TextReader::value( const char* name, double& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = attribute->real();
    }
}

void TextReader::value( const char* name, char* value, size_t max )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        strncpy( value, attribute->string().c_str(), max );
        value[max - 1] = '\0';
    }
}

void TextReader::value( const char* name, wchar_t* value, size_t max )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        Ucs2CharFilter char_filter;
        size_t utf8_length = attribute->string().length();
        size_t ucs2_length = std::min( char_filter.to_memory_length(attribute->string().c_str(), attribute->string().c_str() + utf8_length), max );
        char_filter.to_memory( attribute->string().c_str(), attribute->string().c_str() + utf8_length, value, value + ucs2_length );
        value[max - 1] = L'\0';
    }
}

void TextReader::value( const char* name, std::string& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = attribute->string();
    }
}

void TextReader::value( const char* name, std::wstring& value )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        Ucs2CharFilter char_filter;
        size_t utf8_length = attribute->string().length();
        size_t ucs2_length = char_filter.to_memory_length( attribute->string().c_str(), attribute->string().c_str() + utf8_length );
        if ( ucs2_length > 0 )
        {
            std::vector<wchar_t> ucs2_buffer( ucs2_length, L'\0' );
            char_filter.to_memory( attribute->string().c_str(), attribute->string().c_str() + utf8_length, &ucs2_buffer[0], &ucs2_buffer[0] + ucs2_length );
            value.assign( &ucs2_buffer[0], ucs2_length );
        }
    }
}

Element* TextReader::find_element( const std::string& name )
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( m_state.top().m_element );

    Element* element = NULL;
    if ( !get_sequence() )
    {
        element = m_state.top().m_element->find_element( name );
    }
    else
    {
        SWEET_ASSERT( m_state.top().m_position != m_state.top().m_element->elements().end() );
        SWEET_ASSERT( m_state.top().m_position->name() == name );
        element = &(*m_state.top().m_position);
    }

    return element;
}

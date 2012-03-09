//
// TextWriter.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "TextWriter.hpp"
#include "Ucs2CharFilter.hpp"
#include "strnlen.hpp"
#include "Writer.ipp"

using namespace sweet::persist;

TextWriter::TextWriter()
: Writer<TextWriter>(),
  m_element(),
  m_state()
{
}

TextWriter::TextWriter( const TextWriter& writer )
: Writer<TextWriter>( writer ),
  m_element(),
  m_state()
{
}

Mode TextWriter::get_mode() const
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_mode;
}

const Element* TextWriter::get_element() const
{
//
// The Element that is returned is actually the first and only child of the
// root Element.  This is because the root Element is a dummy element to 
// avoid a special case when adding the true root Element.
//
    SWEET_ASSERT( m_element.elements().empty() || m_element.elements().size() == 1 );
    return !m_element.elements().empty() ? &m_element.elements().front() : NULL;
}

Element* TextWriter::get_current_element()
{
    SWEET_ASSERT( !m_state.empty() );
    return m_state.top().m_element;
}

void TextWriter::begin_object( const char* name, const void* address, Mode mode, int size )
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( mode == MODE_VALUE || mode == MODE_REFERENCE );
    m_state.push( State(get_current_element()->add_element(Element(name, address)), mode, address) );
}

void TextWriter::end_object()
{
    SWEET_ASSERT( !m_state.empty() );

    const void* address = m_state.top().m_address;
    if ( address && !m_state.top().m_element->is_flag(PERSIST_ANONYMOUS) )
    {
        get_current_element()->add_attribute( Attribute(get_address_keyword(), address) );
    }
    m_state.pop();
    
    SWEET_ASSERT( !m_state.empty() );
}

void TextWriter::type( const std::string& type )
{
    SWEET_ASSERT( !type.empty() );
    value( get_class_keyword().c_str(), const_cast<std::string&>(type) );
}

void TextWriter::flag( int value )
{
    get_current_element()->set_flag( value );
}

void TextWriter::value( const char* name, bool& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, value) );
}

void TextWriter::value( const char* name, char& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<int>(value)) );
}

void TextWriter::value( const char* name, signed char& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<int>(value)) );
}

void TextWriter::value( const char* name, unsigned char& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<unsigned int>(value)) );
}

void TextWriter::value( const char* name, wchar_t& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<unsigned int>(value)) );
}

void TextWriter::value( const char* name, short& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<int>(value)) );
}

void TextWriter::value( const char* name, unsigned short& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<unsigned int>(value)) );
}

void TextWriter::value( const char* name, int& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, value) );
}

void TextWriter::value( const char* name, unsigned int& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<int>(value)) );
}

void TextWriter::value( const char* name, long& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<int>(value)) );
}

void TextWriter::value( const char* name, unsigned long& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<unsigned int>(value)) );
}

#if defined(BUILD_PLATFORM_MSVC)
void TextWriter::value( const char* name, time_t& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<unsigned int>(value)) );
}
#endif

void TextWriter::value( const char* name, float& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, static_cast<double>(value)) );        
}

void TextWriter::value( const char* name, double& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, value) );        
}

void TextWriter::value( const char* name, char* value, size_t max )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, std::string(value, strnlen(value, max))) );
}

void TextWriter::value( const char* name, wchar_t* value, size_t max )
{
    SWEET_ASSERT( name );

    Ucs2CharFilter char_filter;
    size_t ucs2_length = strnlen( value, max );
    size_t utf8_length = char_filter.to_archive_length( value, value + ucs2_length );
    std::vector<char> utf8_buffer( utf8_length, '\0' );
    char_filter.to_archive( value, value + ucs2_length, &utf8_buffer[0], &utf8_buffer[0] + utf8_length );
    get_current_element()->add_attribute( Attribute(name, std::string(&utf8_buffer[0], utf8_length)) );
}

void TextWriter::value( const char* name, std::string& value )
{
    SWEET_ASSERT( name );
    get_current_element()->add_attribute( Attribute(name, value) );
}

void TextWriter::value( const char* name, std::wstring& value )
{
    SWEET_ASSERT( name );

    if ( !value.empty() )
    {
        Ucs2CharFilter char_filter;
        size_t ucs2_length = value.length();
        size_t utf8_length = char_filter.to_archive_length( value.c_str(), value.c_str() + ucs2_length );
        std::vector<char> utf8_buffer( utf8_length, '\0' );
        char_filter.to_archive( value.c_str(), value.c_str() + ucs2_length, &utf8_buffer[0], &utf8_buffer[0] + utf8_length );
        get_current_element()->add_attribute( Attribute(name, std::string(&utf8_buffer[0], utf8_length)) );
    }
    else
    {
        get_current_element()->add_attribute( Attribute(name, std::string()) );
    }
}


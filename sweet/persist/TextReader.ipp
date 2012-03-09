//
// TextReader.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_TEXTREADER_IPP_INCLUDED
#define SWEET_PERSIST_TEXTREADER_IPP_INCLUDED

#include "Resolver.hpp"
#include "TextReader.hpp"
#include "Reader.ipp"
#include "objects.ipp"
#include <string.h>

namespace sweet
{

namespace persist
{

template <class Type>
void TextReader::read( const char* name, Type& object )
{
    m_resolver.set_filename( get_filename() );
    m_resolver.set_contexts( get_contexts() );
    reset();

    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(&m_element, MODE_VALUE) );
    load( *this, MODE_VALUE, name, object );
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );

    m_resolver.process( version(), object );
}

template <class Type>
void TextReader::read( const char* name, const char* child_name, Type& container )
{
    m_resolver.set_filename( get_filename() );
    m_resolver.set_contexts( get_contexts() );
    reset();

    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(&m_element, MODE_VALUE) );
    load( *this, MODE_VALUE, name, child_name, container );
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );

    m_resolver.process( version(), container );
}

template <class Type, size_t LENGTH>
void TextReader::read( const char* name, const char* child_name, Type (&values)[LENGTH] )
{
    m_resolver.set_filename( get_filename() );
    m_resolver.set_contexts( get_contexts() );

    reset();

    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(&m_element, MODE_VALUE) );
    load( *this, MODE_VALUE, name, child_name, values, LENGTH );
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );

    m_resolver.process( version(), values, LENGTH );
}

template <class Type> 
void TextReader::enter( const char* format, int version, Type& object )
{
    Reader::enter( format, version, object );
    m_resolver.enter( format, version, object );
}

template <class Type> 
void 
TextReader::declare( const char* name, int flags )
{
    Reader::declare<Type>( name, flags );
    m_resolver.declare<Type>( name, flags );
}

template <class Filter> 
void TextReader::value( const char* name, wchar_t* value, size_t max, const Filter& filter )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        std::wstring filtered_value = filter.to_memory( widen(attribute->string()) );
        wcsncpy( value, filtered_value.c_str(), max );
        value[max - 1] = L'\0';
    }
}

template <class Filter> 
void TextReader::value( const char* name, std::wstring& value, const Filter& filter )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = filter.to_memory( widen(attribute->string()) );
    }
}

template <class Filter> 
void TextReader::value( const char* name, char* value, size_t max, const Filter& filter )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        std::string filtered_value = filter.to_memory( attribute->string() );
        strncpy( value, filtered_value.c_str(), max );
        value[max - 1] = '\0';
    }
}

template <class Filter> 
void TextReader::value( const char* name, std::string& value, const Filter& filter )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = filter.to_memory( attribute->string() );
    }
}

template <class Type, class Filter> 
void TextReader::value( const char* name, Type& value, const Filter& filter )
{
    Attribute* attribute = get_current_element()->find_attribute( name );
    if ( attribute )
    {
        value = static_cast<Type>( filter.to_memory(attribute->string()) );
    }
}

template <class Type> 
void TextReader::value( const char* name, Type& object )
{
    load( *this, MODE_VALUE, name, object );
}

template <class Type> 
void TextReader::refer( const char* name, Type& object )
{
    load( *this, MODE_REFERENCE, name, object );
}

template <class Type> 
void TextReader::value( const char* name, const char* child_name, Type& container )
{
    load( *this, MODE_VALUE, name, child_name, container );
}

template <class Type> 
void TextReader::refer( const char* name, const char* child_name, Type& container )
{
    load( *this, MODE_REFERENCE, name, child_name, container );
}

template <class Type, size_t LENGTH> 
void TextReader::value( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    load( *this, MODE_VALUE, name, child_name, values, LENGTH );
}

template <class Type, size_t LENGTH> 
void TextReader::refer( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    load( *this, MODE_REFERENCE, name, child_name, values, LENGTH );
}

}

}

#endif


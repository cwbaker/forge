//
// Resolver.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_RESOLVER_IPP_INCLUDED
#define SWEET_PERSIST_RESOLVER_IPP_INCLUDED

#include "Resolver.hpp"
#include "Error.hpp"

namespace sweet
{

namespace persist
{

template <class Type>
void Resolver::process( int version, Type& object )
{
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE, &m_object) );
    set_version( version );
    resolve( *this, MODE_VALUE, object );
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );

    if ( !m_references.empty() )
    {
        SWEET_ERROR( UnresolvedReferencesError(m_references, "Not all references were resolved") );
    }
}

template <class Type> 
void Resolver::process( int version, const char* child_name, Type& container )
{
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE, &m_object) );
    set_version( version );
    resolve( *this, MODE_VALUE, container );
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );

    if ( !m_references.empty() )
    {
        SWEET_ERROR( UnresolvedReferencesError(m_references, "Not all references were resolved") );
    }
}

template <class Type> 
void Resolver::process( int version, const char* child_name, Type& values, size_t length )
{
    SWEET_ASSERT( m_state.empty() );
    m_state.push( State(MODE_VALUE, &m_object) );
    set_version( version );
    resolve( *this, MODE_VALUE, values, length );
    m_state.pop();
    SWEET_ASSERT( m_state.empty() );

    if ( !m_references.empty() )
    {
        SWEET_ERROR( UnresolvedReferencesError(m_references, "Not all references were resolved") );
    }
}

template <class Filter> 
void Resolver::value( const char* name, wchar_t* value, size_t max, const Filter& filter )
{
}

template <class Filter> 
void Resolver::value( const char* name, std::wstring& value, const Filter& filter )
{
}

template <class Filter> 
void Resolver::value( const char* name, char* value, size_t max, const Filter& filter )
{
}

template <class Filter> 
void Resolver::value( const char* name, std::string& value, const Filter& filter )
{
}

template <class Type, class Filter> 
void Resolver::value( const char* name, Type& value, const Filter& filter )
{
}

template <class Type> 
void Resolver::value( const char* name, Type& object )
{
    resolve( *this, MODE_VALUE, object );
}

template <class Type> 
void Resolver::refer( const char* name, Type& object )
{
    resolve( *this, MODE_REFERENCE, object );
}

template <class Type> 
void Resolver::value( const char* name, const char* child_name, Type& container )
{
    resolve( *this, MODE_VALUE, container );
}

template <class Type> 
void Resolver::refer( const char* name, const char* child_name, Type& container )
{
    resolve( *this, MODE_REFERENCE, container );
}

template <class Type, size_t LENGTH> 
void Resolver::value( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    resolve( *this, MODE_VALUE, values, LENGTH );
}

template <class Type, size_t LENGTH> 
void Resolver::refer( const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    resolve( *this, MODE_REFERENCE, values, LENGTH );
}

}

}

#endif

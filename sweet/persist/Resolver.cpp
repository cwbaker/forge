//
// Resolver.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Resolver.hpp"
#include "Writer.ipp"

using std::set;
using std::list;
using namespace sweet::persist;

Resolver::Resolver()
: Writer<Resolver>( ARCHIVE_RESOLVER ),
  m_state(),
  m_object( NULL ),
  m_current_object(),
  m_addresses(),
  m_references()
{
    m_current_object.push( &m_object );
}

Resolver::Resolver( const Resolver& resolver )
: Writer<Resolver>( resolver ),
  m_state(),
  m_object( NULL ),
  m_current_object(),
  m_addresses(),
  m_references()
{
}

void Resolver::track( void* raw_ptr, void* smart_ptr )
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( m_state.top().m_object );

    const void* address = m_state.top().m_object->m_address;
    
    std::multiset<Reference>::iterator reference = m_references.find( Reference(address, 0, 0) );
    while ( reference != m_references.end() && reference->address() == address )
    {
        reference->resolve( raw_ptr, smart_ptr );
        m_references.erase( reference++ );
    }
    
    m_addresses.insert( Address(address, raw_ptr, smart_ptr) );
}

void Resolver::reference( const void* ignored_address, void* reference, void (*resolve)(void* reference, void* raw_ptr, void* smart_ptr) )
{
    SWEET_ASSERT( m_state.top().m_object );
    SWEET_ASSERT( m_state.top().m_current_reference_address != m_state.top().m_object->m_reference_addresses.end() );

    const void* actual_address = *m_state.top().m_current_reference_address;
    ++m_state.top().m_current_reference_address;

    if ( actual_address )
    {
        set<Address>::iterator address = m_addresses.find( Address(actual_address, 0, 0) );
        if ( address != m_addresses.end() )
        {
            if ( resolve )
            {
                (*resolve)( reference, address->raw_ptr(), address->smart_ptr() );
            }
        }
        else
        {
            m_references.insert( Reference(actual_address, reference, resolve) );
        }
    }
}

Mode Resolver::get_mode() const
{
//
// The Mode from the state stack is returned if it is not empty otherwise
// MODE_VALUE is returned.  MODE_VALUE is returned so that the base 
// Writer::enter() function will record the version correctly (if 
// Writer::enter() is called while an Archive is in MODE_REFERENCE then
// it ignores the format and version).
//
    return !m_state.empty() ? m_state.top().m_mode : MODE_VALUE;
}

int Resolver::get_count()
{
    SWEET_ASSERT( !m_state.empty() );

    int count = 0;
    if ( m_state.top().m_object != 0 )
    {
        count = static_cast<int>( m_state.top().m_object->m_reference_addresses.size() );
    }
    return count;
}

void Resolver::begin_reference_addresses( const void* address )
{
    SWEET_ASSERT( !m_current_object.empty() && m_current_object.top() );
    Object* current_object = m_current_object.top();
    current_object->m_objects.push_back( Object(address) );
    m_current_object.push( &current_object->m_objects.back() );
}

void Resolver::end_reference_addresses()
{
    SWEET_ASSERT( !m_current_object.empty() );
    m_current_object.pop();
}

void Resolver::move_reference_addresses_backward( int n )
{
//
// This moves the reference identifiers from the last object that was read in 
// back to before the reference identifiers that were read before the previous
// \e n objects.
//
// This is used when an object is read into an ordered sequence out of order.
// This may happen when someone has edited the attributes that affect the
// order in a text editor or if the ordering mechanism has changed in the 
// application code.  Allowing the reference identifiers to be reordered here 
// ensures that they will be encountered in the correct order on the resolve 
// pass.
//
    SWEET_ASSERT( n > 0 );
    SWEET_ASSERT( !m_current_object.empty() );

    Object* current_object = m_current_object.top();
    list<Object>::iterator to = current_object->m_objects.end();
    list<Object>::iterator from = current_object->m_objects.end();
    std::advance( to, -n );
    std::advance( from, -1 );
    current_object->m_objects.splice( to, current_object->m_objects, from );
}

void Resolver::add_reference_address( const void* address )
{
    SWEET_ASSERT( !m_current_object.empty() && m_current_object.top() );
    m_current_object.top()->m_reference_addresses.push_back( address );
}

void Resolver::begin_object( const char* name, const void* address, Mode mode, int size )
{
    SWEET_ASSERT( mode == MODE_VALUE || mode == MODE_REFERENCE );

    if ( mode == MODE_VALUE )
    {
        const Object* object = 0;
        if ( m_state.top().m_object )
        {
            SWEET_ASSERT( m_state.top().m_current_object != m_state.top().m_object->m_objects.end() );
            object = &(*m_state.top().m_current_object);
            ++m_state.top().m_current_object;
        }

        m_state.push( State(mode, object) );
    }
    else
    {
        SWEET_ASSERT( !m_state.empty() );
        m_state.top().m_mode = MODE_REFERENCE;
    }
}

void Resolver::end_object()
{
    if ( m_state.top().m_mode == MODE_VALUE )
    {
        m_state.pop();
    }
    else
    {
        m_state.top().m_mode = MODE_VALUE;
    }
}

bool Resolver::is_object() const
{
    SWEET_ASSERT( !m_state.empty() );
    SWEET_ASSERT( m_state.top().m_object != 0 );
    return m_state.top().m_object->m_address != 0;
}

void Resolver::type( const std::string& type )
{
}

void Resolver::flag( int value )
{
}

void Resolver::value( const char* name, bool& value )
{
}

void Resolver::value( const char* name, char& value )
{
}

void Resolver::value( const char* name, signed char& value )
{
}

void Resolver::value( const char* name, unsigned char& value )
{
}

void Resolver::value( const char* name, wchar_t& value )
{
}

void Resolver::value( const char* name, short& value )
{
}

void Resolver::value( const char* name, unsigned short& value )
{
}

void Resolver::value( const char* name, int& value )
{
}

void Resolver::value( const char* name, unsigned int& value )
{
}

void Resolver::value( const char* name, long& value )
{
}

void Resolver::value( const char* name, unsigned long& value )
{
}

#if defined(BUILD_PLATFORM_MSVC)
void Resolver::value( const char* name, std::time_t& value )
{
}
#endif

void Resolver::value( const char* name, float& value )
{
}

void Resolver::value( const char* name, double& value )
{
}

void Resolver::value( const char* name, wchar_t* value, size_t max )
{
}

void Resolver::value( const char* name, std::wstring& value )
{
}

void Resolver::value( const char* name, char* value, size_t max )
{
}

void Resolver::value( const char* name, std::string& value )
{
}

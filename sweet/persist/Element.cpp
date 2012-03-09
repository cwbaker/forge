//
// Element.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Element.hpp"

using namespace sweet::persist;

Element::Element()
: m_name(),
  m_address( 0 ),
  m_flags( 0 ),
  m_attributes(),
  m_elements()
{
}

Element::Element( const std::string& name, const void* address )
: m_name( name ),
  m_address( address ),
  m_flags( 0 ),
  m_attributes(),
  m_elements()
{
}

const std::string& Element::name() const
{
    return m_name;
}

const void* Element::address() const
{
    return m_address;
}

bool Element::is_leaf() const
{
    std::list<Element>::const_iterator i = m_elements.begin();
    while ( i != m_elements.end() && i->elements().empty() && i->attributes().empty() )
    {
        ++i;
    }

    return i == m_elements.end();
}

void Element::set_flag( int flag )
{
    m_flags |= flag;
}

bool Element::is_flag( int flag ) const
{
    return (m_flags & flag) != 0;
}

std::list<Attribute>& Element::attributes()
{
    return m_attributes;
}

const std::list<Attribute>& Element::attributes() const
{
    return m_attributes;
}

std::list<Element>& Element::elements()
{
    return m_elements;
}

const std::list<Element>& Element::elements() const
{
    return m_elements;
}

void Element::clear()
{
    m_name.clear();
    m_attributes.clear();
    m_elements.clear();
}

Element* Element::add_element( const Element& element )
{
    m_elements.push_back( element );
    return &m_elements.back();
}

void Element::add_attribute( const Attribute& attribute )
{
    m_attributes.push_back( attribute );
}

Attribute* Element::find_attribute( const std::string& name )
{
    std::list<Attribute>::iterator i = m_attributes.begin();
    while ( i != m_attributes.end() && i->name() != name )
    {
        ++i;
    }
    return i != m_attributes.end() ? &(*i) : NULL;
}

Element* Element::find_element( const std::string& name )
{
    std::list<Element>::iterator i = m_elements.begin();
    while ( i != m_elements.end() && i->name() != name )
    {
        ++i;
    }
    return i != m_elements.end() ? &(*i) : NULL;
}

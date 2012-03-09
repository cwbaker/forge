//
// Item.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/persist/persist.hpp>
#include "Item.hpp"

using namespace sweet::persist;

const EnumFilter::Conversion sweet::persist::ITEM_TYPES[] =
{
    { ITEM_TYPE_NORMAL, "Normal" },
    { ITEM_TYPE_SMALL, "Small" },
    { ITEM_TYPE_LARGE, "Large" },
    { 0, 0 }
};

const MaskFilter::Conversion sweet::persist::ITEM_FLAGS[] =
{
    { ITEM_FLAG_SELECTED, "SELECTED" },
    { ITEM_FLAG_EXPANDED, "EXPANDED" },
    { ITEM_FLAG_DIRTY, "DIRTY" },
    { ITEM_FLAG_NONE, "NONE" },
    { 0, 0 }
};

Item::Item()
: m_string(),
  m_wstring(),
  m_boolean( false ), 
  m_integer( 0 ),
  m_real( 0.0f ),
  m_type( ITEM_TYPE_NORMAL ),
  m_flags( ITEM_FLAG_NONE ),
  m_references( 0 )
{
}

Item::Item( const std::string& string, int integer )
: m_string( string ),
  m_wstring(),
  m_boolean( false ), 
  m_integer( integer ),
  m_real( 0.0f ),
  m_type( ITEM_TYPE_NORMAL ),
  m_flags( ITEM_FLAG_NONE ),
  m_references( 0 )
{
}

Item& Item::operator=( const Item& item )
{
    if ( this != &item )
    {
        m_string = item.m_string;
        m_boolean = item.m_boolean;
        m_integer = item.m_integer;
        m_real = item.m_real;
        m_type = item.m_type;
        m_flags = item.m_flags;
    }    
    return *this;
}

void Item::clear()
{
    m_string.clear();
    m_wstring.clear();
    m_boolean = false;
    m_integer = 0;
    m_real = 0.0f;
    m_type = ITEM_TYPE_NORMAL;
    m_flags = ITEM_FLAG_NONE;
}

bool Item::operator==( const Item& item ) const
{
    return m_string == item.m_string 
        && m_wstring == item.m_wstring 
        && m_boolean == item.m_boolean 
        && m_integer == item.m_integer 
        && m_type == item.m_type 
        && m_flags == item.m_flags
        ;
}

bool Item::operator!=( const Item& item ) const
{
    return !(*this == item);
}

bool Item::operator<( const Item& item ) const
{
    return m_string < item.m_string;
}

Item::operator size_t() const
{
	size_t val = 2166136261U;
    std::string::const_iterator i = m_string.begin();
	while ( i != m_string.end() )
    {
		val = 16777619U * val ^ static_cast<size_t>( *i );
        ++i;
    }
	return val;    
}

void sweet::persist::intrusive_ptr_add_ref( Item* item )
{
    SWEET_ASSERT( item );
    ++item->m_references;
}

void sweet::persist::intrusive_ptr_release( Item* item )
{
    SWEET_ASSERT( item );
    --item->m_references;
    if ( item->m_references <= 0 )
    {
        delete item;
    }
}

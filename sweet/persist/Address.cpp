//
// Address.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Address.hpp"

using namespace sweet::persist;

Address::Address( const void* address, void* raw_ptr, void* smart_ptr )
: m_address( address ),
  m_raw_ptr( raw_ptr ),
  m_smart_ptr( smart_ptr )
{
}

const void* Address::address() const
{
    return m_address;
}

void* Address::raw_ptr() const
{
    return m_raw_ptr;
}

void* Address::smart_ptr() const
{
    return m_smart_ptr;
}

bool Address::operator<( const Address& address ) const
{
    return m_address < address.m_address;
}

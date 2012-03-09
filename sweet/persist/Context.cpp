//
// Context.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Context.hpp"

using namespace sweet;
using namespace sweet::persist;

Context::Context( const rtti::Type& type, void* context )
: m_type( type ),
  m_context( context )
{
}

void* Context::get_context() const
{
    return m_context;
}

bool Context::operator<( const Context& context ) const
{
    return m_type < context.m_type;
}

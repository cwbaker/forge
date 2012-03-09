//
// Mutex.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Mutex.hpp"
#include <sweet/assert/assert.hpp>

using namespace sweet::thread;

/**
// Constructor.
*/
Mutex::Mutex()
: m_locked( false )
{
    ::InitializeCriticalSection( &m_critical_section );
}

/**
// Destructor.
*/
Mutex::~Mutex()
{
    SWEET_ASSERT( !m_locked );
    ::DeleteCriticalSection( &m_critical_section );
}

/**
// Lock this Mutex.
*/
void Mutex::lock()
{
    ::EnterCriticalSection( &m_critical_section );
    m_locked = true;
}

/**
// Unlock this Mutex.
*/
void Mutex::unlock()
{
    m_locked = false;
    ::LeaveCriticalSection( &m_critical_section );
}

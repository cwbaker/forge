//
// Condition.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Condition.hpp"
#include "ScopedLock.hpp"
#include <sweet/assert/assert.hpp>
#include <limits.h>
#include <windows.h>

using namespace sweet::thread;

/**
// Constructor.
*/
Condition::Condition()
: m_mutex(),
  m_queue( ::CreateSemaphore(0, 0, INT_MAX, 0) ),
  m_waiting( 0 )
{
    SWEET_ASSERT( m_queue );
}

/**
// Destructor.
*/
Condition::~Condition()
{
    if ( m_queue != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( m_queue );
        m_queue = INVALID_HANDLE_VALUE;
    }
}

/**
// Wait on this Condition.
//
// @param lock
//  The ScopedLock to unlock while waiting and lock when notified (assumed to 
//  be already locked).
*/
void Condition::wait( ScopedLock& lock )
{
    m_mutex.lock();
    ++m_waiting;
    m_mutex.unlock();

    lock.unlock();

    DWORD result = ::WaitForSingleObject( m_queue, INFINITE );
    SWEET_ASSERT( result == WAIT_OBJECT_0 );

    m_mutex.lock();
    --m_waiting;
    m_mutex.unlock();

    lock.lock();
}

/**
// Notify a single thread that is waiting on this Condition.
//
// If no threads are waiting then this function has no effect.
*/
void Condition::notify_one()
{
    m_mutex.lock();
    int waiting = m_waiting;
    m_mutex.unlock();

    if ( waiting > 0 )
    {
        DWORD result = ::ReleaseSemaphore( m_queue, 1, 0 );
        SWEET_ASSERT( result != 0 );
    }
}

/**
// Notify all threads waiting on this Condition.
*/
void Condition::notify_all()
{
    m_mutex.lock();
    int waiting = m_waiting;
    m_mutex.unlock();

    if ( waiting > 0 )
    {
        DWORD result = ::ReleaseSemaphore( m_queue, waiting, 0 );
        SWEET_ASSERT( result != 0 );
    }
}


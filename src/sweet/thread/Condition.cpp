//
// Condition.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Condition.hpp"
#include "ScopedLock.hpp"
#include <sweet/assert/assert.hpp>
#include <limits.h>

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#endif

using namespace sweet::thread;

/**
// Constructor.
*/
Condition::Condition()
#if defined(BUILD_OS_WINDOWS)
: m_mutex(),
  m_queue( ::CreateSemaphore(0, 0, INT_MAX, 0) ),
  m_waiting( 0 )
#else
  //condition_()
#endif  
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( m_queue );

#else
    int result = pthread_cond_init( &condition_, NULL );
    SWEET_ASSERT( result == 0 );
    (void) result;
#endif
}

/**
// Destructor.
*/
Condition::~Condition()
{
#if defined(BUILD_OS_WINDOWS)
    if ( m_queue != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( m_queue );
        m_queue = INVALID_HANDLE_VALUE;
    }

#else
    pthread_cond_destroy( &condition_ );
#endif
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
#if defined(BUILD_OS_WINDOWS)
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

#else
    pthread_cond_wait( &condition_, lock.pthread_mutex() );
#endif
}

/**
// Notify a single thread that is waiting on this Condition.
//
// If no threads are waiting then this function has no effect.
*/
void Condition::notify_one()
{
#if defined(BUILD_OS_WINDOWS)
    m_mutex.lock();
    int waiting = m_waiting;
    m_mutex.unlock();

    if ( waiting > 0 )
    {
        DWORD result = ::ReleaseSemaphore( m_queue, 1, 0 );
        SWEET_ASSERT( result != 0 );
    }

#else
    int result = pthread_cond_signal( &condition_ );
    SWEET_ASSERT( result == 0 );
    (void) result;
#endif
}

/**
// Notify all threads waiting on this Condition.
*/
void Condition::notify_all()
{
#if defined(BUILD_OS_WINDOWS)
    m_mutex.lock();
    int waiting = m_waiting;
    m_mutex.unlock();

    if ( waiting > 0 )
    {
        DWORD result = ::ReleaseSemaphore( m_queue, waiting, 0 );
        SWEET_ASSERT( result != 0 );
    }

#else
    int result = pthread_cond_broadcast( &condition_ );
    SWEET_ASSERT( result == 0 );
    (void) result;
#endif
}

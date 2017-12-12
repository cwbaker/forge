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
#if defined(BUILD_OS_WINDOWS)
: m_locked( false )
#endif
{
#if defined(BUILD_OS_WINDOWS)
    ::InitializeCriticalSection( &m_critical_section );
#elif defined(BUILD_OS_MACOSX)
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &mutex_, &attr );
    pthread_mutexattr_destroy( &attr );
#endif
}

/**
// Destructor.
*/
Mutex::~Mutex()
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( !m_locked );
    ::DeleteCriticalSection( &m_critical_section );
#elif defined(BUILD_OS_MACOSX)
    pthread_mutex_destroy( &mutex_ );
#endif
}

/**
// Lock this Mutex.
*/
void Mutex::lock()
{
#if defined(BUILD_OS_WINDOWS)
    ::EnterCriticalSection( &m_critical_section );
    m_locked = true;
#elif defined(BUILD_OS_MACOSX)
    pthread_mutex_lock( &mutex_ );
#endif
}

/**
// Unlock this Mutex.
*/
void Mutex::unlock()
{
#if defined(BUILD_OS_WINDOWS)
    m_locked = false;
    ::LeaveCriticalSection( &m_critical_section );
#elif defined(BUILD_OS_MACOSX)
    pthread_mutex_unlock( &mutex_ );
#endif
}

#if defined(BUILD_OS_MACOSX)
pthread_mutex_t* Mutex::pthread_mutex()
{
    return &mutex_;
}
#endif

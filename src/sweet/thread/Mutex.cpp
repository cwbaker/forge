//
// Mutex.cpp
// Copyright (c) 2008 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Mutex.hpp"
#include <sweet/assert/assert.hpp>

using namespace sweet::thread;

/**
// Constructor.
*/
Mutex::Mutex()
{
#ifdef BUILD_OS_WINDOWS
    ::InitializeCriticalSection( &m_critical_section );
#else
    pthread_mutex_init( &mutex_, NULL );
#endif
}

/**
// Destructor.
*/
Mutex::~Mutex()
{
#ifdef BUILD_OS_WINDOWS
    ::DeleteCriticalSection( &m_critical_section );
#else
    pthread_mutex_destroy( &mutex_ );
#endif
}

/**
// Lock this Mutex.
*/
void Mutex::lock()
{
#ifdef BUILD_OS_WINDOWS
    ::EnterCriticalSection( &m_critical_section );
#else
    pthread_mutex_lock( &mutex_ );
#endif
}

/**
// Unlock this Mutex.
*/
void Mutex::unlock()
{
#ifdef BUILD_OS_WINDOWS
    ::LeaveCriticalSection( &m_critical_section );
#else
    pthread_mutex_unlock( &mutex_ );
#endif
}

#ifndef BUILD_OS_WINDOWS
pthread_mutex_t* Mutex::pthread_mutex()
{
    return &mutex_;
}
#endif

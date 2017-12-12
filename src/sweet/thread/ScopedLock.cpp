//
// ScopedLock.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ScopedLock.hpp"
#include "Mutex.hpp"

using namespace sweet::thread;

/**
// Constructor.
//
// @param mutex
//  The Mutex to lock and unlock.
*/
ScopedLock::ScopedLock( Mutex& mutex )
: m_mutex( mutex )
{
    m_mutex.lock();
}

/**
// Destructor.
*/
ScopedLock::~ScopedLock()
{
    m_mutex.unlock();
}

/**
// Lock the Mutex associated with this ScopedLock.
*/
void ScopedLock::lock()
{
    m_mutex.lock();
}

/**
// Unlock the Mutex associated with this ScopedLock.
*/
void ScopedLock::unlock()
{
    m_mutex.unlock();
}

#if defined(BUILD_OS_MACOSX)
pthread_mutex_t* ScopedLock::pthread_mutex() const
{
    return m_mutex.pthread_mutex();
}
#endif

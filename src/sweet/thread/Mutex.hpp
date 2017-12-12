//
// Mutex.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_MUTEX_HPP_INCLUDED
#define SWEET_THREAD_MUTEX_HPP_INCLUDED

#include "declspec.hpp"

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOSX)
#include <pthread.h>
#endif

namespace sweet
{

namespace thread
{

/**
// A mutex.
*/
class SWEET_THREAD_DECLSPEC Mutex
{
#if defined(BUILD_OS_WINDOWS)
    CRITICAL_SECTION m_critical_section; ///< The Windows CRITICAL_SECTION that is used to implement the mutex.
    bool m_locked; ///< Whether or not this Mutex is locked.
#elif defined(BUILD_OS_MACOSX)
    pthread_mutex_t mutex_;
#endif

public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();

#if defined(BUILD_OS_MACOSX)
    pthread_mutex_t* pthread_mutex();
#endif

private:
    Mutex( const Mutex& mutex );
    Mutex& operator=( const Mutex& mutex );
};

}

}

#endif

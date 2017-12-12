//
// ScopedLock.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_SCOPEDLOCK_HPP_INCLUDED
#define SWEET_THREAD_SCOPEDLOCK_HPP_INCLUDED

#include "declspec.hpp"

#if defined(BUILD_OS_MACOSX)
#include <pthread.h>
#endif

namespace sweet
{

namespace thread
{

class Mutex;

/**
// Associate a scope with locking and unlocking a Mutex.
*/
class SWEET_THREAD_DECLSPEC ScopedLock
{
    Mutex& m_mutex;

    public:
        ScopedLock( Mutex& mutex );
        ~ScopedLock();
        void lock();
        void unlock();

#if defined(BUILD_OS_MACOSX)
        pthread_mutex_t* pthread_mutex() const;
#endif

    private:
        ScopedLock( const ScopedLock& lock );
        ScopedLock& operator=( const ScopedLock& lock );
};

}

}

#endif

#ifndef SWEET_THREAD_SCOPEDLOCK_HPP_INCLUDED
#define SWEET_THREAD_SCOPEDLOCK_HPP_INCLUDED

#include "declspec.hpp"

#if !defined(BUILD_OS_WINDOWS)
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
    bool m_locked;

    public:
        ScopedLock( Mutex& mutex );
        ~ScopedLock();
        void lock();
        void unlock();

#if !defined(BUILD_OS_WINDOWS)
        pthread_mutex_t* pthread_mutex() const;
#endif

    private:
        ScopedLock( const ScopedLock& lock );
        ScopedLock& operator=( const ScopedLock& lock );
};

}

}

#endif

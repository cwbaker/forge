//
// Condition.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_CONDITION_HPP_INCLUDED
#define SWEET_THREAD_CONDITION_HPP_INCLUDED

#include "declspec.hpp"

#if defined(BUILD_OS_WINDOWS)
#include "Mutex.hpp"
#elif defined(BUILD_OS_MACOSX)
#include <pthread.h>
#endif

namespace sweet
{

namespace thread
{

class ScopedLock;

/**
// A condition variable.
*/
class SWEET_THREAD_DECLSPEC Condition
{
#if defined(BUILD_OS_WINDOWS)
    Mutex m_mutex; ///< Ensures exclusive access to this Condition.
    void* m_queue; ///< The queue semaphore.
    unsigned int m_waiting; ///< The number of threads waiting on this Condition.

#elif defined(BUILD_OS_MACOSX)
    pthread_cond_t condition_;
#endif

    public:
        Condition();
        ~Condition();

        void wait( ScopedLock& lock );
        void notify_one();
        void notify_all();

    private:
        Condition( const Condition& condition );
        Condition& operator=( const Condition& condition );
};

}

}

#endif

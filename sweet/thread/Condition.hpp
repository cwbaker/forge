//
// Condition.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_CONDITION_HPP_INCLUDED
#define SWEET_THREAD_CONDITION_HPP_INCLUDED

#include "declspec.hpp"
#include "Mutex.hpp"

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
    Mutex m_mutex; ///< Ensures exclusive access to this Condition.
    void* m_queue; ///< The queue semaphore.
    unsigned int m_waiting; ///< The number of threads waiting on this Condition.

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

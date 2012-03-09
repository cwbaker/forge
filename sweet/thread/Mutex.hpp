//
// Mutex.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_MUTEX_HPP_INCLUDED
#define SWEET_THREAD_MUTEX_HPP_INCLUDED

#include "declspec.hpp"
#include <windows.h>

namespace sweet
{

namespace thread
{

/**
// A mutex.
*/
class SWEET_THREAD_DECLSPEC Mutex
{
    CRITICAL_SECTION m_critical_section; ///< The Windows CRITICAL_SECTION that is used to implement the mutex.
    bool m_locked; ///< Whether or not this Mutex is locked.

    public:
        Mutex();
        ~Mutex();
        void lock();
        void unlock();

    private:
        Mutex( const Mutex& mutex );
        Mutex& operator=( const Mutex& mutex );
};

}

}

#endif

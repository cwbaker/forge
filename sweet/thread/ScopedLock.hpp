//
// ScopedLock.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_SCOPEDLOCK_HPP_INCLUDED
#define SWEET_THREAD_SCOPEDLOCK_HPP_INCLUDED

#include "declspec.hpp"

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

    private:
        ScopedLock( const ScopedLock& lock );
        ScopedLock& operator=( const ScopedLock& lock );
};

}

}

#endif

#ifndef SWEET_THREAD_MUTEX_HPP_INCLUDED
#define SWEET_THREAD_MUTEX_HPP_INCLUDED

#include "declspec.hpp"

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#else
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
#ifdef BUILD_OS_WINDOWS
    CRITICAL_SECTION m_critical_section; ///< The Windows CRITICAL_SECTION that is used to implement this Mutex.
#else
    pthread_mutex_t mutex_; ///< The POSIX mutex used to implement this Mutex.
#endif

public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();

#ifndef BUILD_OS_WINDOWS
    pthread_mutex_t* pthread_mutex();
#endif

private:
    Mutex( const Mutex& mutex );
    Mutex& operator=( const Mutex& mutex );
};

}

}

#endif

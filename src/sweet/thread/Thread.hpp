//
// Thread.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_THREAD_HPP_INCLUDED
#define SWEET_THREAD_THREAD_HPP_INCLUDED

#include "declspec.hpp"

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOSX) || defined(BUILD_OS_ANDROID)
#include <pthread.h>
#endif

namespace sweet
{

namespace thread
{

/**
// An operating system thread.
*/
class SWEET_THREAD_DECLSPEC Thread
{
public:
    typedef int (*ThreadFunction)( void* context ); ///< The function signature of a thread function.

private:
#if defined(BUILD_OS_WINDOWS)
    void* m_thread; ///< The Windows HANDLE to the thread.
#elif defined(BUILD_OS_MACOSX) || defined(BUILD_OS_ANDROID)
    pthread_t thread_;
    int exit_code_;
#endif
    int m_thread_id; ///< The identifier of the thread.
    ThreadFunction m_function; ///< The entry point function for the thread.
    void* m_context; ///< The context to pass to the entry point function.

public:
    Thread( ThreadFunction function, void* context );
    ~Thread();
    int get_thread_id() const;
    int exit_code() const;
    bool join( int timeout );

private:
    Thread( const Thread& thread );
    Thread& operator=( Thread& thread );

#if defined(BUILD_OS_WINDOWS)
    static DWORD WINAPI thread_entry_point( LPVOID context );
#elif defined(BUILD_OS_MACOSX)
    static void* thread_entry_point( void* context );
#endif
};

}

}

#endif

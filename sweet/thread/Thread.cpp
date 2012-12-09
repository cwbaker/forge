//
// Thread.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Thread.hpp"
#include "Error.hpp"
#include <sweet/error/macros.hpp>
#include <sweet/assert/assert.hpp>

using namespace sweet::thread;

/**
// Constructor.
//
// @param function
//  The function to start executing in the new thread.
//
// @param context
//  The context to pass to the thread function.
*/
Thread::Thread( int (*function)(void*), void* context )
#if defined(BUILD_OS_WINDOWS)
: m_thread( NULL ),
#elif defined(BUILD_OS_MACOSX)
: //thread_(),
  exit_code_( 0 ),
#endif
  m_thread_id( 0 ),
  m_function( function ),
  m_context( context )
{
#if defined(BUILD_OS_WINDOWS)
    DWORD thread_id = 0;
    m_thread = ::CreateThread( NULL, 0, &Thread::thread_entry_point, this, 0, &thread_id );
    if ( !m_thread )
    {
        char error [512];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( CreatingThreadFailedError("Creating a thread failed - %s", error) );
    }

    m_thread_id = static_cast<int>( thread_id );

#elif defined(BUILD_OS_MACOSX)
    int result = pthread_create( &thread_, NULL, &Thread::thread_entry_point, this );
    if ( result != 0 )
    {
        SWEET_ERROR( CreatingThreadFailedError("Creating a thread failed - result=%d", result) );
    }
#endif
}

/**
// Destructor.
*/
Thread::~Thread()
{
#if defined(BUILD_OS_WINDOWS)
    if ( m_thread )
    {
        ::CloseHandle( m_thread );
        m_thread = NULL;
        m_thread_id = 0;
    }

#elif defined(BUILD_OS_MACOSX)
    pthread_detach( thread_ );
#endif
}

/**
// Get the thread id of this Thread.
//
// @return
//  The thread id or 0 if this Thread has been detached.
*/
int Thread::get_thread_id() const
{
    return m_thread_id;
}

/**
// Get the exit code of this Thread.
//
// Assumes that this Thread has finished execution and that Thread::join()
// has been successfully called (and returned true).
//
// @return
//  The exit code.
*/
int Thread::exit_code() const
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( m_thread && m_thread_id != 0 );

    DWORD exit_code = 0;
    BOOL result = ::GetExitCodeThread( m_thread, &exit_code );
    if ( !result )
    {
        char error [512];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( ExitCodeThreadFailedError("Getting exit code for thread %d failed - %s", m_thread_id, error) );
    }
    return static_cast<int>( exit_code );

#elif defined(BUILD_OS_MACOSX)
    return exit_code_;
#endif
}

/**
// Join this Thread.
//
// @param timeout
//  The amount of time to wait for the Thread to finish (in milliseconds) or 0
//  to wait without timing out.
//
// @return
//  True if this Thread finished or is finished otherwise false.
*/
bool Thread::join( int timeout )
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( m_thread && m_thread_id != 0 );

    DWORD result = ::WaitForSingleObject( m_thread, timeout != 0 ? timeout : INFINITE );
    if ( result == WAIT_FAILED )
    {
        char error [512];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( JoiningThreadFailedError("Joining thread %d failed - %s", m_thread_id, error) );
    }
    return result == WAIT_OBJECT_0;

#elif defined(BUILD_OS_MACOSX)
    SWEET_ASSERT( timeout == 0 );

    void* value = NULL;
    int result = pthread_join( thread_, &value );
    if ( result != 0 )
    {
        SWEET_ERROR( JoiningThreadFailedError("Joining thread %d failed - result=%d", result) );
    }
    return true;
#endif
}

#if defined(BUILD_OS_WINDOWS)
/**
// The operating system thread entry point.
//
// @param context
//  The ThreadThunker that provides information about the function that the 
//  call should be diverted to.
//
// @return
//  The result of the ThreadFunction.
*/
DWORD WINAPI Thread::thread_entry_point( LPVOID context )
{
    Thread* thread = reinterpret_cast<Thread*>( context );
    SWEET_ASSERT( thread );
    return static_cast<DWORD>( (*thread->m_function)(thread->m_context) );
}
#endif

#if defined(BUILD_OS_MACOSX)
void* Thread::thread_entry_point( void* context )
{
    Thread* thread = reinterpret_cast<Thread*>( context );
    SWEET_ASSERT( thread );
    return reinterpret_cast<void*>( (*thread->m_function)(thread->m_context) );
}
#endif

//
// Thread.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_THREAD_HPP_INCLUDED
#define SWEET_THREAD_THREAD_HPP_INCLUDED

#include "declspec.hpp"
#include <windows.h>

namespace sweet
{

namespace thread
{

class ThreadThunker;

/**
// An operating system thread.
*/
class SWEET_THREAD_DECLSPEC Thread
{
    public:
        typedef int (*ThreadFunction)( void* context ); ///< The function signature of a thread function.

    private:
        void* m_thread; ///< The Windows HANDLE to the thread.
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
        static DWORD WINAPI thread_entry_point( LPVOID context );
        Thread( const Thread& thread );
        Thread& operator=( Thread& thread );
};

}

}

#endif

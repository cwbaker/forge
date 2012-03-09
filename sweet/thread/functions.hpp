//
// functions.hpp
// Copyright (c) 2011 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_THREAD_FUNCTIONS_HPP_INCLUDED
#define SWEET_THREAD_FUNCTIONS_HPP_INCLUDED

namespace sweet
{

namespace thread
{

/**
// Get the thread id of the calling thread.
//
// @return
//  The thread id of the calling thread.
*/
inline int current_thread_id()
{
#if defined(BUILD_PLATFORM_MSVC) || defined(BUILD_PLATFORM_MINGW)
    return static_cast<int>( ::GetCurrentThreadId() );
#else
#error "The function 'current_thread_id()' is not implemented for this platform."
#endif
}

}

}
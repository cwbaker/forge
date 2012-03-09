//
// atomic.ipp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_ATOMIC_ATOMIC_IPP_INCLUDED
#define SWEET_ATOMIC_ATOMIC_IPP_INCLUDED

#include <sweet/assert/assert.hpp>

#if defined(BUILD_PLATFORM_MSVC)
#include <windows.h>
#endif

namespace sweet
{

namespace atomic
{

/**
// Perform an atomic increment.
//
// @param destination
//  The value in memory to increment.
//
// @return
//  The resulting incremented value.
*/
inline int atomic_increment( volatile int* destination )
{
#if defined(BUILD_PLATFORM_MSVC)
    SWEET_ASSERT( destination );
    return static_cast<int>( ::InterlockedIncrement(reinterpret_cast<volatile long*>(destination)) );
#else
#error "The function 'sweet::atomic::atomic_increment()' is not implemened for this platform."
#endif   
}

/**
// Perform an atomic decrement.
//
// @param destination
//  The value in memory to decrement.
//
// @return
//  The resulting decremented value.
*/
inline int atomic_decrement( volatile int* destination )
{
#if defined(BUILD_PLATFORM_MSVC)
    SWEET_ASSERT( destination );
    return static_cast<int>( ::InterlockedDecrement(reinterpret_cast<volatile long*>(destination)) );
#else
#error "The function 'sweet::atomic::atomic_decrement()' is not implemened for this platform."
#endif   
}

/**
// Perfom an atomic exchange.
//
// @param destination
//  The value in memory to exchange with.
//
// @param exchange
//  The value to exchange with the value at \e destination.
//
// @return
//  The original value at \e destination.
*/
inline int atomic_exchange( volatile int* destination, int exchange )
{
#if defined(BUILD_PLATFORM_MSVC)
    SWEET_ASSERT( destination );
    return static_cast<int>( ::InterlockedExchange(reinterpret_cast<volatile long*>(destination), exchange) );
#else
#error "The function 'sweet::atomic::atomic_exchange()' is not implemened for this platform."
#endif   
}

/**
// Perform an atomic comparison and exchange.
//
// If the value at \e destination equals \e comparand then the value at 
// \e destination is exchanged with \e exchange.
//
@code
int original_value = *value;
if ( *value == comparand )
{
    *value = exchange;
}
return original_value;
@endcode
//
// The \e destination pointer is assumed not to be null and to be aligned on 
// a 32 bit address (i.e. <tt>(\e value & 0x03) == 0</tt>).
//
// @param destination
//  The value to exchange with.
//
// @param exchange
//  The value to exchange with the value at \e destination.
//
// @param comparand
//  The value to compare with the value at \e destination.
//
// @return
//  The original value at \e value.
*/ 
inline int atomic_compare_exchange( volatile int* destination, int exchange, int comparand )
{
#if defined(BUILD_PLATFORM_MSVC)
    SWEET_ASSERT( destination );
    return static_cast<int>( ::InterlockedCompareExchange(reinterpret_cast<volatile long*>(destination), exchange, comparand) );
#else
#error "The function 'sweet::atomic::atomic_compare_exchange()' is not implemened for this platform."
#endif   
}

}

}

#endif

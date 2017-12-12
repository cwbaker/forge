//
// ErrorPolicy.cpp
// Copyright (c) 2001 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ErrorPolicy.hpp"
#include "ErrorPolicyEventSink.hpp"
#include <sweet/assert/assert.hpp>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if defined(BUILD_PLATFORM_MSVC)
#define vsnprintf _vsnprintf
#endif

using namespace sweet::error;

/**
// Constructor.
*/
ErrorPolicy::ErrorPolicy( ErrorPolicyEventSink* event_sink )
: event_sink_( event_sink ),
  stack_index_( 0 )
  //errors_( 0 )
{
    memset( errors_, 0, sizeof(errors_) );
}

/**
// Push an error count onto the stack of error counts.
//
// Assumes that the no more than \e ERROR_STACK_SIZE (8) levels are ever
// pushed.
*/
void ErrorPolicy::push_errors()
{
    SWEET_ASSERT( stack_index_ >= 0 && stack_index_ < ERROR_STACK_SIZE - 1 );
    ++stack_index_;
    errors_[stack_index_] = 0;
}

/**
// Pop an error count from the stack of error counts.
//
// The number of errors at the popped level is added to the previous level 
// and returned so that application code can branch based on errors happening
// or not in different sections of code.
//
// @return
//  The number of errors that have been counted at the popped error count 
//  level.
*/
int ErrorPolicy::pop_errors()
{
    SWEET_ASSERT( stack_index_ > 0  );
    int errors = errors_[stack_index_];
    --stack_index_;
    errors_[stack_index_] += errors;
    return errors;
}

/**
// Get the number of errors that have occured at the current level of the
// stack of error counts.
//
// @return
//  The number of erros counted at the current error count level.
*/
int ErrorPolicy::errors() const
{
    return errors_[stack_index_];
}

/**
// Count an error if \e condition is true; otherwise ignore it.
//
// If \e condition is true then the error count at the current stack level is
// incremented.  If this %ErrorPolicy has an event sink then it is notified 
// that an error has occured through a call to ErrorPolicyEventSink::error().
//
// @param condition
//  True to indicate that an error has occured false otherwise.
//
// @param format
//  A printf style format string that desribes the error that has occured.
//
// @param ...
//  Variable arguments matching the format specified in \e format.
*/
void ErrorPolicy::error( bool condition, const char* format, ... )
{
    if ( condition )
    {
        ++errors_[stack_index_];
        
        char message [1024];
        va_list args;
        va_start( args, format );
        vsnprintf( message, sizeof(message), format, args );
        va_end( args );
        
        if ( event_sink_ )
        {
            event_sink_->error( message );
        }
        else
        {
            fputs( message, stderr );
            fputs( ".\n", stderr );
        }
    }
}

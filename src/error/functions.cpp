//
// functions.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Error.hpp"
#include "ErrorPolicy.hpp"
#include "functions.hpp"
#include <assert/assert.hpp>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(BUILD_OS_MACOS) || defined(BUILD_OS_LINUX)
#include <signal.h>
#endif

using namespace sweet::error;

namespace sweet
{

namespace error
{

/**
// @internal
//
// The global ErrorPolicy that sweet::error::error() reports Errors to.
*/
static ErrorPolicy* sweet_error_policy = NULL;

/**
// Set the global ErrorPolicy.
//
// @param error_policy
//  The ErrorPolicy to set as the global ErrorPolicy or null to set the global
//  ErrorPolicy to nothing.
*/
void set_error_policy( ErrorPolicy* error_policy )
{
    sweet_error_policy = error_policy;
}

/**
// Get the global ErrorPolicy.
//
// @return
//  The global ErrorPolicy or null if the global ErrorPolicy hasn't been set.
*/
ErrorPolicy* get_error_policy()
{
    return sweet_error_policy;
}

/**
// Handle an error.
//
// Passes the error on to the global ErrorPolicy if one has been set otherwise
// prints the error description to stderr and calls ::exit() passing
// EXIT_FAILURE as the return code to pass back to the operating system.
//
// @param error
//  The Error object that describes the error that has occured.
*/
void error( const Error& error )
{
    ErrorPolicy* error_policy = sweet::error::get_error_policy();
    if ( error_policy )
    {
        error_policy->error( true, error.what() );
    }
    else
    {
        fputs( error.what(), stderr );
        fputs( ".\n", stderr );
        exit( EXIT_FAILURE );
    }
}

/**
// Handle an error.
//
// Passes the error on to the global ErrorPolicy if one has been set otherwise
// prints the error description to stderr and calls ::exit() passing
// EXIT_FAILURE as the return code to pass back to the operating system.
//
// @param exception
//  The std::exception object that describes the error that has occured.
*/
void error( const std::exception& exception )
{
    ErrorPolicy* error_policy = sweet::error::get_error_policy();
    if ( error_policy )
    {
        error_policy->error( true, exception.what() );
    }
    else
    {
        fputs( exception.what(), stderr );
        fputs( ".\n", stderr );
        exit( EXIT_FAILURE );
    }
}

/**
// Format an operating system error message.
//
// @param error
//  The operating system error number.
//
// @param buffer
//  A buffer to place the operating system error message into.
//
// @param length
//  The length of the buffer.
//
// @return
//  The buffer.
*/
const char* format( int error, char* buffer, unsigned int length )
{
    SWEET_ASSERT( buffer );
#if defined(BUILD_OS_WINDOWS)
    int actual_length = ::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, buffer, static_cast<int>(length), 0 );
    while ( actual_length > 0 && (buffer[actual_length] == '\n' || buffer[actual_length] == '\r' || buffer[actual_length] == '.' || buffer[actual_length] == 0) )
    {
        buffer[actual_length] = 0;
        --actual_length;
    }
#elif defined(BUILD_OS_MACOS)
    strerror_r( error, buffer, length );
#else
    (void) error;
    (void) buffer;
    (void) length;
#endif
    return buffer;
}

/**
// Get the symbolic name of a POSIX signal.
//
// @param signal
//  The signal number.
//
// @return
//  The signal name (e.g. "SIGSEGV") or "unknown signal" if the signal is not
//  recognised.
*/
const char* signal_name( int signal )
{
#if defined(BUILD_OS_MACOS) || defined(BUILD_OS_LINUX)
    switch ( signal )
    {
        case SIGHUP:
            return "SIGHUP";

        case SIGINT:
            return "SIGINT";

        case SIGQUIT:
            return "SIGQUIT";

        case SIGILL:
            return "SIGILL";

        case SIGTRAP:
            return "SIGTRAP";

        case SIGABRT:
            return "SIGABRT";

        case SIGFPE:
            return "SIGFPE";

        case SIGKILL:
            return "SIGKILL";

        case SIGBUS:
            return "SIGBUS";

        case SIGSEGV:
            return "SIGSEGV";

        case SIGSYS:
            return "SIGSYS";

        case SIGPIPE:
            return "SIGPIPE";

        case SIGALRM:
            return "SIGALRM";

        case SIGTERM:
            return "SIGTERM";

        case SIGURG:
            return "SIGURG";

        case SIGSTOP:
            return "SIGSTOP";

        case SIGTSTP:
            return "SIGTSTP";

        case SIGCONT:
            return "SIGCONT";

        case SIGCHLD:
            return "SIGCHLD";

        case SIGTTIN:
            return "SIGTTIN";

        case SIGTTOU:
            return "SIGTTOU";

        case SIGXCPU:
            return "SIGXCPU";

        case SIGXFSZ:
            return "SIGXFSZ";

        case SIGVTALRM:
            return "SIGVTALRM";

        case SIGPROF:
            return "SIGPROF";

        case SIGWINCH:
            return "SIGWINCH";

        case SIGUSR1:
            return "SIGUSR1";

        case SIGUSR2:
            return "SIGUSR2";

        default:
            return "unknown signal";
    }
#else
    (void) signal;
    return "unknown signal";
#endif
}

/**
// Get the symbolic name of a Windows NTSTATUS exception code.
//
// @param code
//  The NTSTATUS exception code.
//
// @return
//  The exception name (e.g. "EXCEPTION_ACCESS_VIOLATION") or
//  "unknown exception" if the code is not recognised.
*/
const char* exception_name( unsigned long code )
{
#if defined(BUILD_OS_WINDOWS)
    switch ( code )
    {
        case EXCEPTION_ACCESS_VIOLATION:
            return "EXCEPTION_ACCESS_VIOLATION";

        case EXCEPTION_DATATYPE_MISALIGNMENT:
            return "EXCEPTION_DATATYPE_MISALIGNMENT";

        case EXCEPTION_BREAKPOINT:
            return "EXCEPTION_BREAKPOINT";

        case EXCEPTION_SINGLE_STEP:
            return "EXCEPTION_SINGLE_STEP";

        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";

        case EXCEPTION_FLT_DENORMAL_OPERAND:
            return "EXCEPTION_FLT_DENORMAL_OPERAND";

        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            return "EXCEPTION_FLT_DIVIDE_BY_ZERO";

        case EXCEPTION_FLT_INEXACT_RESULT:
            return "EXCEPTION_FLT_INEXACT_RESULT";

        case EXCEPTION_FLT_INVALID_OPERATION:
            return "EXCEPTION_FLT_INVALID_OPERATION";

        case EXCEPTION_FLT_OVERFLOW:
            return "EXCEPTION_FLT_OVERFLOW";

        case EXCEPTION_FLT_STACK_CHECK:
            return "EXCEPTION_FLT_STACK_CHECK";

        case EXCEPTION_FLT_UNDERFLOW:
            return "EXCEPTION_FLT_UNDERFLOW";

        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            return "EXCEPTION_INT_DIVIDE_BY_ZERO";

        case EXCEPTION_INT_OVERFLOW:
            return "EXCEPTION_INT_OVERFLOW";

        case EXCEPTION_PRIV_INSTRUCTION:
            return "EXCEPTION_PRIV_INSTRUCTION";

        case EXCEPTION_IN_PAGE_ERROR:
            return "EXCEPTION_IN_PAGE_ERROR";

        case EXCEPTION_ILLEGAL_INSTRUCTION:
            return "EXCEPTION_ILLEGAL_INSTRUCTION";

        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            return "EXCEPTION_NONCONTINUABLE_EXCEPTION";

        case EXCEPTION_STACK_OVERFLOW:
            return "EXCEPTION_STACK_OVERFLOW";

        case EXCEPTION_INVALID_DISPOSITION:
            return "EXCEPTION_INVALID_DISPOSITION";

        case EXCEPTION_GUARD_PAGE:
            return "EXCEPTION_GUARD_PAGE";

        case EXCEPTION_INVALID_HANDLE:
            return "EXCEPTION_INVALID_HANDLE";

        case STATUS_CONTROL_C_EXIT:
            return "STATUS_CONTROL_C_EXIT";

        default:
            return "unknown exception";
    }
#else
    (void) code;
    return "unknown exception";
#endif
}

}

}

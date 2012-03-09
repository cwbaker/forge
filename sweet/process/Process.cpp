//
// Process.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved
//

#include "stdafx.hpp"
#include "Process.hpp"
#include "Error.hpp"
#include <sweet/assert/assert.hpp>
#include <windows.h>

using namespace sweet::process;

/**
// Constructor.
*/
Process::Process()
: flags_( PROCESS_FLAG_NONE ),
  process_( INVALID_HANDLE_VALUE ),
  stdout_( INVALID_HANDLE_VALUE )
{
}

/**
// Constructor.
//
// @param command
//  The command to execute for this Process (assumed not null).
//
// @param arguments
//  The arguments to pass to execute this Process (assumed not null).
//
// @param directory
//  The directory to execute this Process in (assumed not null).
//
// @param flags
//  A bitwise or of flags that indicates the features that this Process 
//  needs to provide.
*/
Process::Process( const char* command, const char* arguments, const char* directory, int flags )
: flags_( flags ),
  process_( INVALID_HANDLE_VALUE ),
  stdout_( INVALID_HANDLE_VALUE )
{
    SWEET_ASSERT( command );
    SWEET_ASSERT( arguments );
    SWEET_ASSERT( directory );

    STARTUPINFO startup_info;
    memset( &startup_info, 0, sizeof(startup_info) );
    startup_info.cb = sizeof(startup_info);

    PROCESS_INFORMATION process_information;
    memset( &process_information, 0, sizeof(process_information) );

//
// If the process flags specified that stdout and stderr should be provided
// to the child process so that the parent process can call Process::read()
// then create pipes that are inherited as stdout and stderr for the child
// process.
//
// A single pipe is created and its read end is marked as not inheritable 
// because the read end is only used by the parent process.  The write end
// is inherited as the pipe that is written to by the child process when it
// writes to stdout.
//
// This pipe is duplicated to provide the handle to the pipe that is written
// to by the child process when it writes to stderr.
//
// The handle to stdin from this process is inherited as the handle to stdin
// for the child process.
//
    HANDLE stdout_read = INVALID_HANDLE_VALUE;
    HANDLE stdout_write = INVALID_HANDLE_VALUE;
    HANDLE stderr_write = INVALID_HANDLE_VALUE;

    if ( flags & PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR )
    {
        SECURITY_ATTRIBUTES security_attributes;
        memset( &security_attributes, 0, sizeof(security_attributes) );
        security_attributes.nLength = sizeof(security_attributes);
        security_attributes.lpSecurityDescriptor = 0;
        security_attributes.bInheritHandle = TRUE;

        BOOL result = ::CreatePipe( &stdout_read, &stdout_write, &security_attributes, 0 );
        if ( !result )
        {
            char error [1024];
            error::Error::format( ::GetLastError(), error, sizeof(error) );
            SWEET_ERROR( CreatingPipeFailedError("Creating stdout for '%s' failed - %s", command, error) );
        }

        ::SetHandleInformation( stdout_read, HANDLE_FLAG_INHERIT, 0 );

        result = ::DuplicateHandle( ::GetCurrentProcess(), stdout_write, ::GetCurrentProcess(), &stderr_write, 0, TRUE, DUPLICATE_SAME_ACCESS );
        if ( !result )
        {
            ::CloseHandle( stdout_read );
            ::CloseHandle( stdout_write );

            char error [1024];
            error::Error::format( ::GetLastError(), error, sizeof(error) );
            SWEET_ERROR( DuplicatingHandleFailedError("Duplicating stdout into stderr for '%s' failed - %s", command, error) );
        }

        startup_info.hStdError = stderr_write;
        startup_info.hStdOutput = stdout_write;
        startup_info.hStdInput = ::GetStdHandle( STD_INPUT_HANDLE );
        startup_info.dwFlags = STARTF_USESTDHANDLES;
    }

//
// Create the process.
//
    BOOL created = ::CreateProcessA( command, const_cast<char*>(arguments), NULL, NULL, TRUE, 0, NULL, directory, &startup_info, &process_information );
    if ( !created )
    {
        ::CloseHandle( stdout_read );
        ::CloseHandle( stdout_write );
        ::CloseHandle( stderr_write );

        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( ExecutingProcessFailedError("Executing '%s' failed - %s", command, error) );
    }

//
// Close the thread handle returned in the process information because it
// isn't used anywhere.
//
    ::CloseHandle( process_information.hThread );

//
// Close the the write ends of the pipes used for stdout and stderr because 
// they only need to be used by the child process and also because calls to 
// ::ReadFile() on the read end only return 0 when all of the write ends have
// been closed.
//
    if ( stdout_write != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( stdout_write );
        stdout_write = INVALID_HANDLE_VALUE;
    }

    if ( stderr_write != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( stderr_write );
        stderr_write = INVALID_HANDLE_VALUE;
    }

    process_ = process_information.hProcess;
    stdout_ = stdout_read;
}

/**
// Destructor.
*/
Process::~Process()
{
    if ( process_ != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( process_ );
        process_ = INVALID_HANDLE_VALUE;
    }

    if ( stdout_ != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( stdout_ );
        stdout_ = INVALID_HANDLE_VALUE;
    }
}

/**
// Wait for this Process to finish.
//
// @return 
//  Nothing.
*/
void Process::wait()
{
    SWEET_ASSERT( process_ != INVALID_HANDLE_VALUE );

    DWORD wait = ::WaitForSingleObject( process_, INFINITE );
    if ( wait != WAIT_OBJECT_0 )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( WaitForProcessFailedError("Waiting for a process failed - %s", error) );
    }
}

/**
// Get the exit code returned by this Process when it exited.
//
// @return
//  The exit code returned by this Process when it exited.
*/
int Process::exit_code()
{
    SWEET_ASSERT( process_ != INVALID_HANDLE_VALUE );

    DWORD exit_code = 0;
    BOOL exited = ::GetExitCodeProcess( process_, &exit_code );
    if ( !exited )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( ExitCodeForProcessFailedError("Getting a process exit code failed - %s", error) );
    }

    return exit_code;
}

/**
// Read from the stdout and stderr of this Process.
//
// If the read on the pipe fails because the write end of the pipe has been
// closed by the child process then this function will return 0.
//
// @param buffer
//  A buffer to receive the read data.
//
// @param length
//  The maximum number of bytes that can be read into \e buffer.
//
// @return
//  The number of bytes read (or 0 if this Process has closed the write
//  end of the pipe or exited).
*/
unsigned int Process::read( void* buffer, unsigned int length )
{
    SWEET_ASSERT( process_ != INVALID_HANDLE_VALUE );
    SWEET_ASSERT( stdout_ != INVALID_HANDLE_VALUE );
    SWEET_ASSERT( flags_ & PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR );

    DWORD read = 0;
    BOOL result = ::ReadFile( stdout_, buffer, length, &read, NULL );
    if ( !result && ::GetLastError() != ERROR_BROKEN_PIPE )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( ReadingPipeFailedError("Reading from a child process failed - %s", error) );
    }

    return read;
}

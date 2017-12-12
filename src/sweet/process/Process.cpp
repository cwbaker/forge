//
// Process.cpp
// Copyright (c) Charles Baker.  All rights reserved
//

#include <stdio.h>
#include "stdafx.hpp"
#include "Process.hpp"
#include "Environment.hpp"
#include "Error.hpp"
#include <sweet/assert/assert.hpp>

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#endif

#if defined(BUILD_OS_MACOSX) || defined(BUILD_OS_LINUX)
#include <sweet/cmdline/Splitter.hpp>
#include <spawn.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#endif

using std::vector;
using namespace sweet::process;

#if defined(BUILD_OS_MACOSX)
extern char* const* environ;
#endif

/**
// Constructor.
*/
Process::Process()
: executable_( NULL ),
  directory_( NULL ),
  environment_( NULL ),
  start_suspended_( false ),
  inherit_environment_( false ),
#if defined(BUILD_OS_WINDOWS)
  process_( INVALID_HANDLE_VALUE ),
  suspended_thread_( INVALID_HANDLE_VALUE )
#elif defined(BUILD_OS_MACOSX)
  process_( 0 ),
  exit_code_( 0 ),
  suspended_( false )
#elif defined(BUILD_OS_LINUX)
  process_( 0 ),
  exit_code_( 0 )
#endif
{
}

/**
// Destructor.
*/
Process::~Process()
{
    resume();

#if defined(BUILD_OS_WINDOWS)   
    if ( process_ != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle( process_ );
        process_ = INVALID_HANDLE_VALUE;
    }

    for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
    {
        if ( pipe->write_fd != (intptr_t) INVALID_HANDLE_VALUE )
        {
            ::CloseHandle( (HANDLE) pipe->write_fd );
            pipe->write_fd = (intptr_t) INVALID_HANDLE_VALUE;
        }
    }

#elif defined(BUILD_OS_MACOSX) || defined(BUILD_OS_LINUX)
    if ( process_ != 0 )
    {
        process_ = 0;
        pid_t result = waitpid( process_, &exit_code_, 0 );
        while ( result < 0 && errno == EINTR )
        {
            result = waitpid( process_, &exit_code_, 0 );
        }
    }

    for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
    {
        if ( pipe->write_fd != -1 )
        {
            close( pipe->write_fd );
            pipe->write_fd = -1;
        }
    }
#endif
}

void Process::executable( const char* executable )
{
    SWEET_ASSERT( executable );
    executable_ = executable;
}

void Process::directory( const char* directory )
{
    directory_ = directory;
}

void Process::environment( const Environment* environment )
{
    environment_ = environment;
}

void Process::start_suspended( bool start_suspended )
{
    start_suspended_ = start_suspended;
}

void Process::inherit_environment( bool inherit_environment )
{
    inherit_environment_ = inherit_environment;
}

/**
// Create a pipe to communicate with the spawned process.
//
// @param child_fd
//  The child file descriptor to dup2() the write end of the pipe into in the
//  child process.
//
// @return
//  The file descriptor for read end of the pipe in the parent process.
*/
intptr_t Process::pipe( int child_fd )
{
    SWEET_ASSERT( child_fd != PIPE_STDIN );

#if defined(BUILD_OS_WINDOWS)
    pipes_.push_back( Pipe() );
    Pipe& pipe = pipes_.back();
    pipe.child_fd = child_fd;
    pipe.read_fd = (intptr_t) INVALID_HANDLE_VALUE;
    pipe.write_fd = (intptr_t) INVALID_HANDLE_VALUE;

    SECURITY_ATTRIBUTES security_attributes;
    memset( &security_attributes, 0, sizeof(security_attributes) );
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.lpSecurityDescriptor = 0;
    security_attributes.bInheritHandle = TRUE;

    HANDLE read_fd = INVALID_HANDLE_VALUE;
    HANDLE write_fd = INVALID_HANDLE_VALUE;
    BOOL result = ::CreatePipe( &read_fd, &write_fd, &security_attributes, 0 );
    if ( !result )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( CreatingPipeFailedError("Creating pipe for '%s' failed - %s", executable_, error) );
    }
    ::SetHandleInformation( read_fd, HANDLE_FLAG_INHERIT, 0 );

    pipe.read_fd = (intptr_t) read_fd;
    pipe.write_fd = (intptr_t) write_fd;
    return pipe.read_fd;

#elif defined(BUILD_OS_MACOSX) || defined(BUILD_OS_LINUX)
    int fds [2] = { -1, -1 };
    int result = ::pipe( fds );
    if ( result != 0 )
    {
        char error [1024];
        error::Error::format( errno, error, sizeof(error) );
        SWEET_ERROR( CreatingPipeFailedError("Creating pipe for '%s' failed - %s", executable_, error) );
    }
    pipes_.push_back( Pipe() );
    Pipe& pipe = pipes_.back();
    pipe.child_fd = child_fd;
    pipe.read_fd = fds[0];
    pipe.write_fd = fds[1];
    return pipe.read_fd;
#endif
}

void Process::run( const char* arguments )
{
    SWEET_ASSERT( executable_ );
    SWEET_ASSERT( arguments );

#if defined(BUILD_OS_WINDOWS)
    STARTUPINFO startup_info;
    memset( &startup_info, 0, sizeof(startup_info) );
    startup_info.cb = sizeof(startup_info);
    startup_info.hStdInput = ::GetStdHandle( STD_INPUT_HANDLE );
    startup_info.hStdOutput = ::GetStdHandle( STD_OUTPUT_HANDLE );
    startup_info.hStdError = ::GetStdHandle( STD_ERROR_HANDLE );
    startup_info.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION process_information;
    memset( &process_information, 0, sizeof(process_information) );

    for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
    {
        switch ( pipe->child_fd )
        {
            case PIPE_STDOUT:
                startup_info.hStdOutput = (HANDLE) pipe->write_fd;
                break;

            case PIPE_STDERR:
                startup_info.hStdError = (HANDLE) pipe->write_fd;
                break;

            default:
                break;
        }
    }

    DWORD creation_flags = 0;
    if ( start_suspended_ )
    {
        creation_flags |= CREATE_SUSPENDED;
    }

    // Create the process.
    void* envp = NULL;
    if ( environment_ )
    {
        envp = (void*) environment_->buffer();
    }
    else if ( !inherit_environment_ )
    {
        const char* empty_environment = "\0\0";
        envp = (void*) empty_environment;
    }
    BOOL created = ::CreateProcessA( executable_, const_cast<char*>(arguments), NULL, NULL, TRUE, creation_flags, envp, directory_, &startup_info, &process_information );
    if ( !created )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( ExecutingProcessFailedError("Executing '%s' failed - %s", executable_, error) );
    }

    // If the process was started suspended then keep its main thread handle
    // open so that it can be resumed later.  Otherwise close the main thread
    // handle as it is not used again.
    if ( start_suspended_ )
    {
        suspended_thread_ = process_information.hThread;
    }
    else
    {
        ::CloseHandle( process_information.hThread );
    }

    // Close the the write ends of any the pipes used because they only need 
    // to be used by the child process and also because calls to ::ReadFile()
    // on the read end only return 0 when all of the write ends have been 
    // closed.
    for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
    {
        if ( pipe->write_fd != (intptr_t) INVALID_HANDLE_VALUE )
        {
            ::CloseHandle( (HANDLE) pipe->write_fd );
            pipe->write_fd = (intptr_t) INVALID_HANDLE_VALUE;
        }
    }

    process_ = process_information.hProcess;

#elif defined(BUILD_OS_MACOSX)
    cmdline::Splitter splitter( arguments );

    if ( directory_ )
    {
        // Use the undocumented `pthread_fchdir()` system call to change the 
        // working directory for the thread that is spawning a process rather than
        // the global per-process working directory changed by `fchdir()`.  See 
        // `syscall()` and `<sys/syscall.h>`.
        int result = syscall( SYS___pthread_chdir, directory_ );
        if ( result != 0 )
        {
            char message [256];
            SWEET_ERROR( ExecutingProcessFailedError("Executing '%s' failed - unable to change directory to '%s' - %s", executable_, directory_, Error::format(errno, message, sizeof(message))) );
        }
    }

    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init( &file_actions );
    for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
    {
        if ( pipe->read_fd < 0 || pipe->write_fd < 0 || pipe->child_fd < 0 )
        {
            char message [256];
            SWEET_ERROR( CreatingPipeFailedError("Creating pipe for '%s' failed - %s", executable_, Error::format(errno, message, sizeof(message))) );
        }
        posix_spawn_file_actions_addclose( &file_actions, pipe->read_fd );
        posix_spawn_file_actions_adddup2( &file_actions, pipe->write_fd, pipe->child_fd );
        posix_spawn_file_actions_addclose( &file_actions, pipe->write_fd );
    }

    posix_spawnattr_t attributes;
    posix_spawnattr_init( &attributes );

    if ( start_suspended_ )
    {
        posix_spawnattr_setflags( &attributes, POSIX_SPAWN_START_SUSPENDED );
        suspended_ = true;
    }

    pid_t pid = 0;
    char* const* envp = NULL;
    if ( inherit_environment_ )
    {
        envp = environ;
    }
    else if ( environment_ )
    {
        envp = environment_->values();
    }
    int result = posix_spawn( &pid, executable_, &file_actions, &attributes, &splitter.arguments()[0], envp );
    posix_spawnattr_destroy( &attributes );
    posix_spawn_file_actions_destroy( &file_actions );

    for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
    {
        close( pipe->write_fd );
        pipe->write_fd = -1;
    }

    if ( result != 0 )
    {
        for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
        {
            close( pipe->read_fd );
            pipe->read_fd = -1;
        }

        char message [256];
        SWEET_ERROR( ExecutingProcessFailedError("Executing '%s' failed - %s", executable_, Error::format(result, message, sizeof(message))) );
    }

    process_ = pid;
#elif defined(BUILD_OS_LINUX)
    process_ = fork();
    if ( process_ == -1 )
    {
        for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
        {
            close( pipe->read_fd );
            pipe->read_fd = -1;
        }

        char message [256];
        SWEET_ERROR( ExecutingProcessFailedError("Fork failed - %s", Error::format(errno, message, sizeof(message))) );
    }
    else if ( process_ == 0 )
    {
        if ( directory_ )
        {
            int result = chdir( directory_ );
            if ( result != 0 )
            {
                char message [256];
                fprintf( stderr, "Changing directory to '%s' failed - %s\n", directory_, Error::format(errno, message, sizeof(message)) );
                _exit( errno );
            }
        }

        for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
        {
            close( pipe->read_fd );
            dup2( pipe->write_fd, pipe->child_fd );
            close( pipe->write_fd );
        }

        cmdline::Splitter splitter( arguments );

        char* const* envp = NULL;
        if ( inherit_environment_ )
        {
            envp = environ;
        }
        else if ( environment_ )
        {
            envp = environment_->values();
        }

        int result = execve( executable_, &splitter.arguments()[0], envp );

        // Ignore any returned result as any call to `execve()` that returns 
        // is a failure.
        (void) result;
        char message [256];
        fprintf( stderr, "Executing '%s' failed - %s\n", executable_, Error::format(errno, message, sizeof(message)) );
        _exit( errno );
    }
    else
    {
        // Close write ends of pipes in the parent process.
        for ( vector<Pipe>::iterator pipe = pipes_.begin(); pipe != pipes_.end(); ++pipe )
        {
            close( pipe->write_fd );
            pipe->write_fd = -1;
        }
    }
#endif    
}

/**
// Get the handle or identifier of this Process.
//
// @return 
//  The handle or identifier of this Process cast to a void pointer.
*/
void* Process::process() const
{
    return (void*) (intptr_t) process_;
}

/**
// Get the handle of the suspended main thread of this Process.
//
// @return
//  The handle to the suspended main thread or null if this Process is not
//  suspended or this code is running on a platform other than Windows.
*/
void* Process::thread() const
{
#if defined(BUILD_OS_WINDOWS)
    return suspended_thread_;
#else
    return NULL;
#endif
}

void* Process::write_pipe( int index ) const
{
#if defined(BUILD_OS_WINDOWS)
    return index >= 0 && index < int(pipes_.size()) ? (void*) pipes_[index].write_fd : INVALID_HANDLE_VALUE;
#else
    (void) index;
    return (void*) -1;
#endif
}

/**
// Resume this Process after it has been started suspended.
//
// It is only valid to call this function once to resume an initially 
// suspended process.  Additional calls will silently do nothing.
*/
void Process::resume()
{
#if defined(BUILD_OS_WINDOWS)
    if ( suspended_thread_ != INVALID_HANDLE_VALUE )
    {
        ::ResumeThread( suspended_thread_ );
        suspended_thread_ = INVALID_HANDLE_VALUE;
    }
#elif defined(BUILD_OS_MACOSX)
    if ( suspended_ )
    {
        suspended_ = false;
        kill( process_, SIGCONT );
    }
#endif
}

/**
// Wait for this Process to finish.
*/
void Process::wait()
{
#if defined(BUILD_OS_WINDOWS)
    SWEET_ASSERT( process_ != INVALID_HANDLE_VALUE );

    DWORD wait = ::WaitForSingleObject( process_, INFINITE );
    if ( wait != WAIT_OBJECT_0 )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( WaitForProcessFailedError("Waiting for a process failed - %s", error) );
    }

#elif defined(BUILD_OS_MACOSX) || defined(BUILD_OS_LINUX)
    SWEET_ASSERT( process_ != 0 );

    pid_t result = waitpid( process_, &exit_code_, 0 );
    while ( result < 0 && errno == EINTR )
    {
        result = waitpid( process_, &exit_code_, 0 );
    }
    if ( result != process_ )
    {
        char buffer [1024];
        SWEET_ERROR( WaitForProcessFailedError("Waiting for a process failed - %s", Error::format(errno, buffer, sizeof(buffer))) );
    }
    process_ = 0;
#endif
}

/**
// Get the exit code returned by this Process when it exited.
//
// @return
//  The exit code returned by this Process when it exited.
*/
int Process::exit_code()
{
#if defined(BUILD_OS_WINDOWS)
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
#elif defined(BUILD_OS_MACOSX) || defined(BUILD_OS_LINUX)
    SWEET_ASSERT( process_ == 0 );
    return exit_code_;
#endif
}

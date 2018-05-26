//
// Reader.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "Reader.hpp"
#include "Scheduler.hpp"
#include "BuildTool.hpp"
#include "Error.hpp"
#include <sweet/error/Error.hpp>
#include <sweet/assert/assert.hpp>
#include <stdlib.h>
#include <memory>

#if defined(BUILD_OS_WINDOWS)
#include <windows.h>
#elif defined(BUILD_OS_MACOS)
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#endif

using std::find;
using std::string;
using std::vector;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::build_tool;

Reader::Reader( BuildTool* build_tool )
: build_tool_( build_tool ),
  jobs_mutex_(),
  jobs_empty_condition_(),
  jobs_ready_condition_(),
  jobs_(),
  threads_(),
  active_jobs_( 0 ),
  done_( false )
{
}

Reader::~Reader()
{
    stop();
}

int Reader::active_jobs() const
{
    return active_jobs_;
}

void Reader::read( intptr_t fd_or_handle, Filter* filter, Arguments* arguments, Target* working_directory )
{
    std::unique_lock<std::mutex> lock( jobs_mutex_ );
    jobs_.push_back( std::bind(&Reader::thread_read, this, fd_or_handle, filter, arguments, working_directory) );
    ++active_jobs_;
    while ( active_jobs_ > int(threads_.size()) )
    {
        unique_ptr<std::thread> thread( new std::thread(&Reader::thread_main, this) );
        threads_.push_back( thread.release() );
    }
    jobs_ready_condition_.notify_all();
}

int Reader::thread_main( void* context )
{
    Reader* reader = reinterpret_cast<Reader*>( context );
    SWEET_ASSERT( reader );
    reader->thread_process();
    return EXIT_SUCCESS;
}

void Reader::thread_process()
{
    std::unique_lock<std::mutex> lock( jobs_mutex_ );
    while ( !done_ )
    {
        if ( !jobs_.empty() )
        {
            std::function<void()> function = jobs_.front();
            jobs_.pop_front();
            lock.unlock();
            function();
            lock.lock();
            --active_jobs_;
        }

        if ( jobs_.empty() )
        {
            jobs_empty_condition_.notify_all();
            jobs_ready_condition_.wait( lock );
        }
    }
}

void Reader::thread_read( intptr_t fd_or_handle, Filter* filter, Arguments* arguments, Target* working_directory )
{
    SWEET_ASSERT( build_tool_ );
    
    try
    {
        char buffer [1024];
        char* pos = buffer;
        char* end = buffer + sizeof(buffer) - 1;

        size_t read = Reader::read( fd_or_handle, pos, end - pos );
        while ( read > 0 )
        {
            char* start = buffer;
            char* finish = pos + read;

            pos = find( start, finish, '\n' );
            while ( pos != finish )
            {
                *pos = 0;
                build_tool_->scheduler()->push_output( string(start, pos), filter, arguments, working_directory );
                start = pos + 1;
                pos = std::find( start, finish, '\n' );
            }

            if ( start > buffer )
            {
                memcpy( buffer, start, finish - start );
            }
            else if ( finish >= end )
            {
                *finish = 0;
                build_tool_->scheduler()->push_output( string(start, finish), filter, arguments, working_directory );
                start = buffer;
                finish = buffer;
            }
            
            pos = buffer + (finish - start);
            read = Reader::read( fd_or_handle, pos, end - pos );
        }

        if ( pos > buffer )
        {
            *pos = 0;
            build_tool_->scheduler()->push_output( string(buffer, pos), filter, arguments, working_directory );
        }

        Reader::close( fd_or_handle );
        build_tool_->scheduler()->push_filter_finished( filter, arguments );
    }

    catch ( const std::exception& exception )
    {
        Scheduler* scheduler = build_tool_->scheduler();
        scheduler->push_error( exception );
        Reader::close( fd_or_handle );
    }
}

void Reader::stop()
{
    if ( !threads_.empty() )
    {
        {
            std::unique_lock<std::mutex> lock( jobs_mutex_ );
            if ( !jobs_.empty() )
            {
                jobs_empty_condition_.wait( lock );
            }
            done_ = true;
            jobs_ready_condition_.notify_all();
        }

        for ( vector<std::thread*>::iterator i = threads_.begin(); i != threads_.end(); ++i )
        {
            try
            {
                std::thread* thread = *i;
                SWEET_ASSERT( thread );
                thread->join();
            }

            catch ( const std::exception& exception )
            {
                build_tool_->error( 0, "Failed to join thread - %s", exception.what() );
            }
        }
        
        while ( !threads_.empty() )
        {
            delete threads_.back();
            threads_.pop_back();
        }
    }
}

/**
// Read up to \e length bytes from the pipe specifed by \e fd_or_handle.
//
// If the read on the pipe fails because the write end of the pipe has been
// closed by the child process then this function will return 0.
//
// @param fd_or_handle
//  The file descriptor or handle to the read end of the pipe to read from.
//
// @param buffer
//  A buffer to receive the read data.
//
// @param length
//  The maximum number of bytes that can be read into \e buffer.
//
// @return
//  The number of bytes read (or 0 if the write end of the pipe is closed and
//  no more data is available).
*/
size_t Reader::read( intptr_t fd_or_handle, void* buffer, size_t length ) const
{
#if defined(BUILD_OS_WINDOWS)
    HANDLE handle = (HANDLE) fd_or_handle;
    SWEET_ASSERT( handle != INVALID_HANDLE_VALUE );

    DWORD read = 0;
    BOOL result = ::ReadFile( handle, buffer, (DWORD) length, &read, NULL );
    if ( !result && ::GetLastError() != ERROR_BROKEN_PIPE )
    {
        char error [1024];
        error::Error::format( ::GetLastError(), error, sizeof(error) );
        SWEET_ERROR( ReadingPipeFailedError("Reading from a child process failed - %s", error) );
    }
    return read;

#elif defined(BUILD_OS_MACOS) || defined(BUILD_OS_LINUX)
    int fd = (int) fd_or_handle;
    SWEET_ASSERT( fd >= 0 );

    int bytes = ::read( fd, buffer, length );
    while ( bytes < 0 && errno == EINTR )
    {
        bytes = ::read( fd, buffer, length );
    }
    if ( bytes == -1 )
    {
        char buffer [1024];
        SWEET_ERROR( ReadingPipeFailedError("Reading from a child process failed - %s", Error::format(errno, buffer, sizeof(buffer))) );        
    }
    return bytes;
#endif
}

void Reader::close( intptr_t fd_or_handle ) const
{
#if defined(BUILD_OS_WINDOWS)
    HANDLE handle = (HANDLE) fd_or_handle;
    ::CloseHandle( handle );
#elif defined(BUILD_OS_MACOS) || defined(BUILD_OS_LINUX)
    int fd = (int) fd_or_handle;
    ::close( fd );
#endif
}

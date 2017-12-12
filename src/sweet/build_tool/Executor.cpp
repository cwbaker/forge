//
// Executor.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Executor.hpp"
#include "Error.hpp"
#include "BuildTool.hpp"
#include "Target.hpp"
#include "Context.hpp"
#include "Scheduler.hpp"
#include <sweet/thread/Thread.hpp>
#include <sweet/thread/ScopedLock.hpp>
#include <sweet/process/Process.hpp>
#include <sweet/process/Environment.hpp>
#include <stdlib.h>

using std::max;
using std::find;
using std::string;
using std::vector;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::process;
using namespace sweet::build_tool;

Executor::Executor( BuildTool* build_tool )
: build_tool_( build_tool ),
  jobs_mutex_(),
  jobs_ready_condition_(),
  jobs_(),
  maximum_parallel_jobs_( 1 ),
  threads_(),
  done_( false )
{
    SWEET_ASSERT( build_tool_ );
}

Executor::~Executor()
{
    stop();
}

void Executor::set_maximum_parallel_jobs( int maximum_parallel_jobs )
{
    stop();
    maximum_parallel_jobs_ = max( 1, maximum_parallel_jobs );
}

int Executor::maximum_parallel_jobs() const
{
    return maximum_parallel_jobs_;
}

void Executor::execute( const std::string& command, const std::string& command_line, process::Environment* environment, lua::LuaValue* filter, Arguments* arguments, Context* context )
{
    SWEET_ASSERT( !command.empty() );
    SWEET_ASSERT( context );

    if ( !command.empty() )
    {
        start();
        thread::ScopedLock lock( jobs_mutex_ );
        jobs_.push_back( std::bind(&Executor::thread_execute, this, command, command_line, environment, filter, arguments, context->working_directory(), context) );
        jobs_ready_condition_.notify_all();
    }
}

int Executor::thread_main( void* context )
{
    Executor* executor = reinterpret_cast<Executor*>( context );
    SWEET_ASSERT( executor );
    executor->thread_process();
    return EXIT_SUCCESS;
}

void Executor::thread_process()
{
    thread::ScopedLock lock( jobs_mutex_ );
    while ( !done_ )
    {
        if ( !jobs_.empty() )
        {
            std::function<void()> function = jobs_.front();
            jobs_.pop_front();
            lock.unlock();
            function();
            lock.lock();
        }

        if ( jobs_.empty() )
        {
            jobs_empty_condition_.notify_all();
            jobs_ready_condition_.wait( lock );
        }
    }
}

void Executor::thread_execute( const std::string& command, const std::string& command_line, process::Environment* environment, lua::LuaValue* filter, Arguments* arguments, Target* working_directory, Context* context )
{
    SWEET_ASSERT( build_tool_ );
    
    try
    {
        char buffer [1024];        
        char* pos = buffer;
        char* end = buffer + sizeof(buffer) - 1;

        Process process( command.c_str(), command_line.c_str(), context->directory().string().c_str(), environment, PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR );
        size_t read = process.read( pos, end - pos );
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
            read = process.read( pos, end - pos );
        }

        if ( pos > buffer )
        {
            *pos = 0;
            build_tool_->scheduler()->push_output( string(buffer, pos), filter, arguments, working_directory );
        }

        process.wait();
        build_tool_->scheduler()->push_execute_finished( process.exit_code(), context, environment, filter, arguments );
    }

    catch ( const std::exception& exception )
    {
        Scheduler* scheduler = build_tool_->scheduler();
        scheduler->push_error( exception, context );
        scheduler->push_execute_finished( EXIT_FAILURE, context, environment, filter, arguments );
    }
}

void Executor::start()
{
    SWEET_ASSERT( maximum_parallel_jobs_ > 0 );

    if ( threads_.empty() )
    {
        thread::ScopedLock lock( jobs_mutex_ );
        done_ = false;
        threads_.reserve( maximum_parallel_jobs_ );
        for ( size_t i = 0; i < maximum_parallel_jobs_; ++i )
        {
            unique_ptr<thread::Thread> thread( new thread::Thread(&Executor::thread_main, this) );
            threads_.push_back( thread.release() );
        }
    }
}

void Executor::stop()
{
    if ( !threads_.empty() )
    {
        {
            thread::ScopedLock lock( jobs_mutex_ );
            if ( !jobs_.empty() )
            {
                jobs_empty_condition_.wait( lock );
            }
            done_ = true;
            jobs_ready_condition_.notify_all();
        }

        for ( vector<thread::Thread*>::iterator i = threads_.begin(); i != threads_.end(); ++i )
        {
            try
            {
                thread::Thread* thread = *i;
                SWEET_ASSERT( thread );
                thread->join( 2000 );
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

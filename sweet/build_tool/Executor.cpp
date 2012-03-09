//
// Executor.cpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Executor.hpp"
#include "Error.hpp"
#include "BuildTool.hpp"
#include "Target.hpp"
#include "Scanner.hpp"
#include "Environment.hpp"
#include "Scheduler.hpp"
#include <sweet/thread/Thread.hpp>
#include <sweet/thread/ScopedLock.hpp>
#include <sweet/process/Process.hpp>
#include <boost/bind.hpp>

using std::max;
using std::find;
using std::string;
using std::vector;
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

int Executor::get_maximum_parallel_jobs() const
{
    return maximum_parallel_jobs_;
}

void Executor::execute( const std::string& command, const std::string& command_line, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Environment> environment )
{
    SWEET_ASSERT( !command.empty() );
    SWEET_ASSERT( environment );

    if ( !command.empty() )
    {
        start();
        thread::ScopedLock lock( jobs_mutex_ );
        jobs_.push_back( boost::bind(&Executor::thread_execute, this, command, command_line, environment->get_directory().string(), scanner, arguments, environment->get_working_directory(), environment) );
        jobs_ready_condition_.notify_all();
    }
}

void Executor::scan( ptr<Target> target, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Environment> environment )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( working_directory );
    SWEET_ASSERT( environment );
    
    if ( target && working_directory )
    {
        start();
        thread::ScopedLock lock( jobs_mutex_ );
        jobs_.push_back( boost::bind(&Executor::thread_scan, this, target, scanner, arguments, working_directory, environment) );
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
            boost::function<void()> function = jobs_.front();
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

void Executor::thread_execute( const std::string& command, const std::string& command_line, const std::string& directory, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Environment> environment )
{
    SWEET_ASSERT( build_tool_ );
    
    try
    {
        process::Process process( command.c_str(), command_line.c_str(), directory.c_str(), PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR );

        char buffer [1024];        
        char* pos = buffer;
        char* end = buffer + sizeof(buffer) - 1;

        size_t read = process.read( pos, end - pos );
        while ( read > 0 )
        {
            char* start = buffer;
            char* finish = pos + read;

            pos = find( start, finish, '\n' );
            while ( pos != finish )
            {
                *pos = 0;
                build_tool_->get_scheduler()->push_output( string(start, pos), scanner, arguments, working_directory );
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
                build_tool_->get_scheduler()->push_output( string(start, finish), scanner, arguments, working_directory );
                start = buffer;
                finish = buffer;
            }
            
            pos = buffer + (finish - start);
            read = process.read( pos, end - pos );
        }

        if ( pos > buffer )
        {
            *pos = 0;
            build_tool_->get_scheduler()->push_output( string(buffer, pos), scanner, arguments, working_directory );
        }

        process.wait();
        build_tool_->get_scheduler()->push_execute_finished( process.exit_code(), environment );
    }

    catch ( const std::exception& exception )
    {
        Scheduler* scheduler = build_tool_->get_scheduler();
        scheduler->push_error( exception, environment );
        scheduler->push_execute_finished( EXIT_FAILURE, environment );
    }
}

void Executor::thread_scan( ptr<Target> target, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Environment> environment )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( scanner );
    SWEET_ASSERT( working_directory );
    SWEET_ASSERT( environment );
    
    Scheduler* scheduler = build_tool_->get_scheduler();
    SWEET_ASSERT( scheduler );
    
    FILE* file = ::fopen( target->get_path().c_str(), "rb" );
    if ( file )
    {
        try
        {
            static const int SCANNER_MAXIMUM_LINE_LENGTH = 1024;
            char buffer [SCANNER_MAXIMUM_LINE_LENGTH + 1];

            int unmatched_lines = 0;
            int maximum_unmatched_lines = scanner->get_initial_lines();

            while ( ::fgets(buffer, sizeof(buffer) - 1, file) != 0 && unmatched_lines < maximum_unmatched_lines )
            {
                buffer [sizeof(buffer) - 1] = '\0';

                bool matched = false;
                const vector<Pattern>& patterns = scanner->get_patterns();
                for ( vector<Pattern>::const_iterator pattern = patterns.begin(); pattern != patterns.end(); ++pattern )
                {
                    boost::match_results<const char*> match;
                    if ( regex_search(buffer, match, pattern->get_regex()) ) 
                    {
                        matched = true;
                        build_tool_->get_scheduler()->push_match( &(*pattern), string(match[1].first, match[1].second), arguments, working_directory, target );
                    }
                }

                if ( matched )
                {
                    unmatched_lines = 0;
                    maximum_unmatched_lines = scanner->get_later_lines();
                }
                else
                {
                    ++unmatched_lines;
                }
            }

            if ( ::ferror(file) != 0 )
            {
                SWEET_ERROR( ScanningFileFailedError("Scanning '%s' failed", target->get_filename().c_str()) );
            }
        }
        
        catch ( const std::exception& exception )
        {
            scheduler->push_error( exception, environment );
            scheduler->push_scan_finished();
        }
        
        ::fclose( file );
        file = NULL;
    }

    scheduler->push_scan_finished();
}

void Executor::start()
{
    SWEET_ASSERT( maximum_parallel_jobs_ > 0 );

    if ( threads_.empty() )
    {
        thread::ScopedLock lock( jobs_mutex_ );
        threads_.reserve( maximum_parallel_jobs_ );
        for ( size_t i = 0; i < maximum_parallel_jobs_; ++i )
        {
            ptr<thread::Thread> thread( new thread::Thread(&Executor::thread_main, this) );
            threads_.push_back( thread );
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

        for ( vector<ptr<thread::Thread> >::iterator i = threads_.begin(); i != threads_.end(); ++i )
        {
            try
            {
                thread::Thread* thread = i->get();
                SWEET_ASSERT( thread );
                thread->join( 2000 );
            }

            catch ( const std::exception& exception )
            {
                build_tool_->error( 0, "Failed to join thread - %s", exception.what() );
            }
        }
        
        threads_.clear();
    }
}

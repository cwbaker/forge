//
// Scheduler.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Scheduler.hpp"
#include "Target.hpp"
#include "Graph.hpp"
#include "BuildTool.hpp"
#include "Job.hpp"
#include "Environment.hpp"
#include "ScriptInterface.hpp"
#include "Executor.hpp"
#include "Pattern.hpp"
#include "Arguments.hpp"
#include "Scanner.hpp"
#include "Error.hpp"
#include <sweet/atomic/atomic.ipp>
#include <sweet/thread/ScopedLock.hpp>
#include <sweet/lua/LuaThread.hpp>
#include <sweet/lua/ptr.hpp>
#include <list>
#include <string>
#include <algorithm>

using std::sort;
using std::list;
using std::vector;
using std::string;
using namespace sweet;
using namespace sweet::atomic;
using namespace sweet::lua;
using namespace sweet::build_tool;

Scheduler::Scheduler( BuildTool* build_tool )
: build_tool_( build_tool ),
  free_environments_(),
  results_mutex_(),
  results_condition_(),
  results_(),
  jobs_( 0 ),
  failures_( 0 ),
  traversing_( false )
{
    SWEET_ASSERT( build_tool_ );
}

Scheduler::~Scheduler()
{
}

void Scheduler::set_traversing( bool traversing )
{
    traversing_ = traversing;
}

bool Scheduler::is_traversing() const
{
    return traversing_;
}

void Scheduler::load( const path::Path& path )
{
    buildfile( path );
    while ( dispatch_results() )
    {        
    }
}

void Scheduler::command( const path::Path& path, const std::string& function )
{
    call( path, function );
    while ( dispatch_results() )
    {
    }
}

void Scheduler::execute( const char* start, const char* finish )
{
    SWEET_ASSERT( start );
    SWEET_ASSERT( finish );
    SWEET_ASSERT( start <= finish );

    Graph* graph = build_tool_->get_graph();
    ScriptInterface* script_interface = build_tool_->get_script_interface();
    ptr<Environment> environment = allocate_environment( graph->target(script_interface->get_initial_directory().string()) );

    process_begin( environment );
    environment->get_environment_thread().resume( start, finish, "BuildTool" )
    .end();
    process_end( environment );

    while ( dispatch_results() )
    {
    }
}

void Scheduler::buildfile( const path::Path& path )
{
    SWEET_ASSERT( path.is_absolute() );

    ptr<Environment> environment = allocate_environment( build_tool_->get_graph()->target(path.branch().string()) );
    process_begin( environment );
    environment->get_environment_thread().resume( path.string().c_str(), path.leaf().c_str() )
    .end();
    process_end( environment );
}

void Scheduler::call( const path::Path& path, const std::string& function )
{
    if ( !function.empty() )
    {
        ptr<Environment> environment = allocate_environment( build_tool_->get_graph()->target(path.branch().string()) );
        process_begin( environment );
        environment->get_environment_thread().resume( function.c_str() )
        .end();
        process_end( environment );
    }
}

void Scheduler::preorder_visit( const lua::LuaValue& function, ptr<Target> target )
{
    SWEET_ASSERT( target );

    ptr<Environment> environment = allocate_environment( target->get_working_directory() );

    process_begin( environment );
    environment->get_environment_thread().resume( function )
        ( target )
    .end();
    
    int errors = process_end( environment );
    if ( errors > 0 )
    {
        ++failures_;
        build_tool_->error( "Preorder visit of '%s' failed", target->get_id().c_str() );
    }
}

void Scheduler::postorder_visit( const lua::LuaValue& function, Job* job )
{
    SWEET_ASSERT( job );

    if ( job->get_target()->is_buildable() )
    {
        ptr<Environment> environment = allocate_environment( job->get_working_directory(), job );
        process_begin( environment );
        environment->get_environment_thread().resume( function )
            ( job->get_target() )
        .end();
        
        int errors = process_end( environment );
        if ( errors > 0 )
        {
            ++failures_;
            build_tool_->error( "Postorder visit of '%s' failed", job->get_target()->get_id().c_str() );
        }

        job->get_target()->set_successful( errors == 0 );
    }
    else
    {
        build_tool_->error( "%s", job->get_target()->generate_failed_dependencies_message().c_str() );
        job->get_target()->set_successful( false );
        job->set_state( JOB_COMPLETE );
    }    
}

void Scheduler::execute_finished( int exit_code, ptr<Environment> environment )
{
    SWEET_ASSERT( environment );

    process_begin( environment );
    environment->get_environment_thread().resume()
        ( exit_code )
    .end();
    process_end( environment );
}

void Scheduler::scan_finished( ptr<Arguments> arguments )
{
    // Reset arguments here on the main thread to avoid accessing the Lua
    // virtual machine from multiple threads as happens if the arguments are
    // destroyed in the worker threads provided by the executor.
    arguments.reset();
}

void Scheduler::output( const std::string& output, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory )
{
    SWEET_ASSERT( build_tool_ );
    if ( scanner )
    {
        int matches = 0;
        const vector<Pattern>& patterns = scanner->get_patterns();
        for ( vector<Pattern>::const_iterator pattern = patterns.begin(); pattern != patterns.end(); ++pattern )
        {
            std::match_results<const char*> match;
            if ( regex_search(output.c_str(), output.c_str() + output.length(), match, pattern->get_regex()) ) 
            {
                ++matches;
                
                ptr<Environment> environment = allocate_environment( working_directory );
                process_begin( environment );
                lua::AddParameter add_parameter = environment->get_environment_thread().call( pattern->get_function() );
                for ( size_t i = 1; i < match.size(); ++i )
                {
                    add_parameter( std::string(match[i].first, match[i].second) );
                }
                if ( arguments )
                {
                    arguments->push_arguments( add_parameter );
                }
                add_parameter.end();                
                process_end( environment );
            }
        }
        
        if ( matches == 0 )
        {
            build_tool_->output( output.c_str() );
        }
    }
    else
    {    
        build_tool_->output( output.c_str() );
    }
}

void Scheduler::match( const Pattern* pattern, ptr<Target> target, const std::string& match, ptr<Arguments> arguments, ptr<Target> working_directory )
{
    SWEET_ASSERT( pattern );
    SWEET_ASSERT( target );
    SWEET_ASSERT( working_directory );
    
    ptr<Environment> environment = allocate_environment( working_directory );

    process_begin( environment );
    AddParameter add_parameter = environment->get_environment_thread().call( pattern->get_function() )
        ( target )
        ( match )
    ;
    if ( arguments )
    {
        arguments->push_arguments( add_parameter );
    }
    add_parameter.end();
    process_end( environment );
}

void Scheduler::error( const std::string& what, ptr<Environment> environment )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( environment );    
    build_tool_->error( what.c_str() );
    build_tool_->get_script_interface()->pop_environment();
    destroy_environment( environment );
}

void Scheduler::push_output( const std::string& output, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory )
{
    thread::ScopedLock lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::output, this, output, scanner, arguments, working_directory) );
    results_condition_.notify_all();
}

void Scheduler::push_error( const std::exception& exception, ptr<Environment> environment )
{
    thread::ScopedLock lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::error, this, string(exception.what()), environment) );
    results_condition_.notify_all();
}

void Scheduler::push_match( const Pattern* pattern, const std::string& match, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Target> target )
{
    thread::ScopedLock lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::match, this, pattern, target, match, arguments, working_directory) );
    results_condition_.notify_all();
}

void Scheduler::push_execute_finished( int exit_code, ptr<Environment> environment )
{
    thread::ScopedLock lock( results_mutex_ );
    --jobs_;
    results_.push_back( std::bind(&Scheduler::execute_finished, this, exit_code, environment) );
    results_condition_.notify_all();
}

void Scheduler::push_scan_finished( ptr<Arguments> arguments )
{
    thread::ScopedLock lock( results_mutex_ );
    --jobs_;
    results_.push_back( std::bind(&Scheduler::scan_finished, this, arguments) );
    results_condition_.notify_all();
}

void Scheduler::execute( const std::string& command, const std::string& command_line, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Environment> environment )
{
    thread::ScopedLock lock( results_mutex_ );
    build_tool_->get_executor()->execute( command, command_line, scanner, arguments, environment );
    ++jobs_;
}

void Scheduler::scan( ptr<Target> target, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Environment> environment )
{
    thread::ScopedLock lock( results_mutex_ );
    build_tool_->get_executor()->scan( target, scanner, arguments, working_directory, environment );
    ++jobs_;
}

void Scheduler::wait()
{
    while ( dispatch_results() )
    {
    }
}

int Scheduler::preorder( const lua::LuaValue& function, ptr<Target> target )
{
    struct Preorder
    {
        BuildTool* build_tool_;
        std::vector<Target*> targets_;
        std::vector<Target*> next_targets_;
    
        Preorder( BuildTool* build_tool )
        : build_tool_( build_tool ),
          targets_(),
          next_targets_()
        {
            SWEET_ASSERT( build_tool_ );
            build_tool_->get_graph()->begin_traversal();
        }
        
        ~Preorder()
        {
            build_tool_->get_graph()->end_traversal();
        }
    
        bool empty() const
        {
            return targets_.empty();
        }

        std::vector<Target*>::const_iterator begin() const
        {
            return targets_.begin();
        }

        std::vector<Target*>::const_iterator end() const
        {
            return targets_.end();
        }

        void start( Target* target )
        {
            targets_.clear();
            targets_.push_back( target );
        }

        void move_to_dependencies()
        {
            unsigned int targets = 0;
            for ( vector<Target*>::const_iterator i = targets_.begin(); i != targets_.end(); ++i )
            {
                Target* target = *i;
                SWEET_ASSERT( target );
                targets += target->get_dependencies().size();
            }

            next_targets_.clear();
            next_targets_.reserve( targets );
            for ( vector<Target*>::const_iterator i = targets_.begin(); i != targets_.end(); ++i )
            {
                Target* target = *i;
                SWEET_ASSERT( target );
                const vector<Target*>& dependencies = target->get_dependencies();
                for ( vector<Target*>::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j )
                {
                    Target* dependency = *j;
                    SWEET_ASSERT( dependency );
                    if ( !dependency->is_visited() )
                    {
                        dependency->set_visited( true );
                        next_targets_.push_back( dependency );
                    }
                }
            }
            targets_.swap( next_targets_ );
        }
    };    

    Graph* graph = build_tool_->get_graph();    
    if ( graph->is_traversal_in_progress() )
    {
        SWEET_ERROR( PreorderCalledRecursivelyError("Preorder called from within another preorder or postorder traversal") );
    }

    failures_ = 0;
    Preorder preorder( build_tool_ );
    preorder.start( target ? target.get() : graph->get_root_target() );

    while ( !preorder.empty() )
    {
        vector<Target*>::const_iterator i = preorder.begin();
        while ( i != preorder.end() )
        {
            Target* target = *i;
            SWEET_ASSERT( target );
            if ( target->is_referenced_by_script() && target->get_working_directory() )
            {
                preorder_visit( function, target->ptr_from_this() );
            }
            ++i;
        }
        wait();
        preorder.move_to_dependencies();
    }

    wait();
    return failures_;
}

int Scheduler::postorder( const lua::LuaValue& function, ptr<Target> target )
{
    struct ScopedVisit
    {
        Target* target_;

        ScopedVisit( Target* target )
        : target_( target )
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( !target_->is_visiting() );
            target_->set_visited( true );
            target_->set_visiting( true );
        }

        ~ScopedVisit()
        {
            SWEET_ASSERT( target_->is_visiting() );
            target_->set_visiting( false );
        }
    };

    struct Postorder
    {
        const lua::LuaValue& function_;
        BuildTool* build_tool_;
        list<Job> jobs_;
        
        Postorder( const lua::LuaValue& function, BuildTool* build_tool )
        : function_( function ),
          build_tool_( build_tool ),
          jobs_()
        {
            SWEET_ASSERT( build_tool_ );
            build_tool_->get_graph()->begin_traversal();
        }
        
        ~Postorder()
        {
            build_tool_->get_graph()->end_traversal();
        }
    
        void remove_complete_jobs()
        {
            list<Job>::iterator job = jobs_.begin();
            while ( job != jobs_.end() )
            {
                if ( job->get_state() == JOB_COMPLETE )
                {
                    job = jobs_.erase( job );
                }
                else
                {
                    ++job;
                }
            }
        }

        Job* pull_job()
        {
            int height = INT_MAX;
            list<Job>::iterator job = jobs_.begin();

            while ( job != jobs_.end() && (job->get_state() != JOB_WAITING || job->get_height() > height) )
            {
                height = std::min( height, job->get_height() );
                ++job;
            }

            if ( job != jobs_.end() )
            {
                SWEET_ASSERT( job->get_state() == JOB_WAITING );
                job->set_state( JOB_PROCESSING );
            }

            return job != jobs_.end() ? &(*job) : NULL;
        }
        
        bool empty() const
        {
            return jobs_.empty();
        }

        void visit( Target* target )
        {
            SWEET_ASSERT( target );

            if ( !target->is_visited() )
            {
                ScopedVisit visit( target );

                int height = 0;
                const vector<Target*>& dependencies = target->get_dependencies();
                for ( vector<Target*>::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i )
                {
                    Target* dependency = *i;
                    SWEET_ASSERT( dependency );

                    if ( !dependency->is_visiting() )
                    {
                        Postorder::visit( dependency );
                        height = std::max( height, dependency->postorder_height() + 1 );
                    }
                    else
                    {
                        build_tool_->warning( "Ignoring cyclic dependency from '%s' to '%s'", target->get_id().c_str(), dependency->get_id().c_str() );
                        dependency->set_successful( true );
                    }
                }

                if ( target->is_referenced_by_script() && target->get_working_directory() )
                {
                    target->set_postorder_height( height );
                    jobs_.push_back( Job(target, height) );
                }
                else
                {
                    target->set_postorder_height( -1 );
                    target->set_successful( true );
                }
            }
        }
    };

    Graph* graph = build_tool_->get_graph();
    if ( graph->is_traversal_in_progress() )
    {
        SWEET_ERROR( PostorderCalledRecursivelyError("Postorder called from within another preorder or postorder traversal") );
    }
    
    failures_ = 0;
    Postorder postorder( function, build_tool_ );
    postorder.visit( target ? target.get() : graph->get_root_target() );

    postorder.remove_complete_jobs();
    while ( !postorder.empty() )
    {
        postorder.remove_complete_jobs();
        Job* job = postorder.pull_job();
        while ( job )
        {
            postorder_visit( function, job );
            postorder.remove_complete_jobs();
            job = postorder.pull_job();
        }
        dispatch_results();
    }

    wait();
    return failures_;
}

ptr<Environment> Scheduler::allocate_environment( ptr<Target> working_directory, Job* job )
{
    SWEET_ASSERT( working_directory );
    SWEET_ASSERT( !job || job->get_working_directory() == working_directory );
    
    if ( free_environments_.empty() )
    {
        const int DEFAULT_ENVIRONMENTS_GROW_BY = 16;
        free_environments_.reserve( free_environments_.size() + DEFAULT_ENVIRONMENTS_GROW_BY );
        for ( int i = 0; i < DEFAULT_ENVIRONMENTS_GROW_BY; ++i )
        {
            ptr<Environment> environment( new Environment(i, path::Path(""), build_tool_) );
            free_environments_.push_back( environment );
        }
    }

    ptr<Environment> environment = free_environments_.back();
    free_environments_.pop_back();
    environment->reset_directory_to_target( working_directory );
    environment->set_job( job );
    return environment;
}

void Scheduler::free_environment( ptr<Environment> environment )
{
    SWEET_ASSERT( environment );
    SWEET_ASSERT( std::find(free_environments_.begin(), free_environments_.end(), environment) == free_environments_.end() );

    Job* job = environment->get_job();
    if ( job )
    {
        job->set_state( JOB_COMPLETE );
    }

    free_environments_.push_back( environment );
    environment->set_job( NULL );
}

void Scheduler::destroy_environment( ptr<Environment> environment )
{
    SWEET_ASSERT( environment );

    Job* job = environment->get_job();
    if ( job )
    {
        job->set_state( JOB_COMPLETE );
        job->get_target()->set_successful( false );
    }
}

bool Scheduler::dispatch_results()
{
    thread::ScopedLock lock( results_mutex_ );
    if ( jobs_ > 0 && results_.empty() )
    {
        results_condition_.wait( lock );
    }

    while ( !results_.empty() )
    {
        std::function<void()> result = results_.front();
        results_.pop_front();
        lock.unlock();
        result();
        lock.lock();
    }
    
    return jobs_ > 0;                 
}

void Scheduler::process_begin( ptr<Environment> environment )
{
    SWEET_ASSERT( environment );
    build_tool_->get_script_interface()->push_environment( environment );
}

int Scheduler::process_end( ptr<Environment> environment )
{
    SWEET_ASSERT( environment );

    int errors = build_tool_->get_script_interface()->pop_environment();
    LuaThreadState state = environment->get_environment_thread().get_state();    

    if ( errors == 0 && state != lua::LUA_THREAD_ERROR )
    {
        if ( state == lua::LUA_THREAD_READY )
        {
            free_environment( environment );
        }
    }
    else
    {
        destroy_environment( environment );
    }
    
    return errors;
}

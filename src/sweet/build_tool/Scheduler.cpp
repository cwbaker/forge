//
// Scheduler.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Scheduler.hpp"
#include "Target.hpp"
#include "Graph.hpp"
#include "BuildTool.hpp"
#include "Job.hpp"
#include "Context.hpp"
#include "Executor.hpp"
#include "Filter.hpp"
#include "Arguments.hpp"
#include "Error.hpp"
#include <sweet/build_tool/build_tool_lua/LuaBuildTool.hpp>
#include <sweet/process/Environment.hpp>
#include <sweet/lua/LuaThreadEventSink.hpp>
#include <sweet/lua/LuaThread.hpp>
#include <list>
#include <string>
#include <memory>
#include <algorithm>

using std::sort;
using std::list;
using std::vector;
using std::string;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

namespace sweet
{

namespace build_tool
{

class BuildfileEventSink : public lua::LuaThreadEventSink
{
    Scheduler* scheduler_;

public:
    BuildfileEventSink( Scheduler* scheduler )
    : scheduler_( scheduler )
    {
        SWEET_ASSERT( scheduler_ );
    }

    void lua_thread_returned( LuaThread* /*thread*/, void* ccontext )
    {
        Context* context = reinterpret_cast<Context*>( ccontext );
        scheduler_->buildfile_finished( context, true );
    }

    void lua_thread_errored( LuaThread* /*thread*/, void* ccontext )
    {
        Context* context = reinterpret_cast<Context*>( ccontext );
        scheduler_->buildfile_finished( context, false );
    }
};

}

}

Scheduler::Scheduler( BuildTool* build_tool )
: build_tool_( build_tool ),
  buildfile_event_sink_( nullptr ),
  active_contexts_(),
  results_mutex_(),
  results_condition_(),
  results_(),
  jobs_( 0 ),
  failures_( 0 )
{
    SWEET_ASSERT( build_tool_ );
    buildfile_event_sink_ = new BuildfileEventSink( this );
}

Scheduler::~Scheduler()
{
    delete buildfile_event_sink_;
}

void Scheduler::load( const boost::filesystem::path& path )
{
    SWEET_ASSERT( path.is_absolute() );

    Context* context = allocate_context( build_tool_->graph()->target(path.parent_path().generic_string()) );
    process_begin( context );
    context->context_thread().resume( path.string().c_str(), path.filename().string().c_str() )
    .end();
    process_end( context );

    while ( dispatch_results() )
    {        
    }
}

void Scheduler::command( const boost::filesystem::path& path, const std::string& function )
{
    call( path, function );
    while ( dispatch_results() )
    {
    }
}

int Scheduler::buildfile( const boost::filesystem::path& path )
{
    SWEET_ASSERT( path.is_absolute() );
    SWEET_ASSERT( !active_contexts_.empty() );

    Context* calling_context = active_contexts_.back();
    Context* context = allocate_context( build_tool_->graph()->target(path.parent_path().generic_string()) );
    process_begin( context );
    context->context_thread().resume( path.string().c_str(), path.filename().string().c_str() )
    .end( buildfile_event_sink_, calling_context );
    bool yielded = context->context_thread().get_state() == LUA_THREAD_SUSPENDED;
    int errors = process_end( context );
    return yielded ? -1 : errors;
}

void Scheduler::call( const boost::filesystem::path& path, const std::string& function )
{
    if ( !function.empty() )
    {
        Context* context = allocate_context( build_tool_->graph()->target(path.parent_path().generic_string()) );
        process_begin( context );
        context->context_thread().resume( function.c_str() )
        .end();
        process_end( context );
    }
}

void Scheduler::postorder_visit( int function, Job* job )
{
    SWEET_ASSERT( job );

    if ( job->target()->buildable() )
    {
        Context* context = allocate_context( job->working_directory(), job );
        process_begin( context );
        context->context_thread().resume( LUA_REGISTRYINDEX, function )
            ( job->target() )
        .end();
        
        int errors = process_end( context );
        if ( errors > 0 )
        {
            ++failures_;
            build_tool_->error( "Postorder visit of '%s' failed", job->target()->id().c_str() );
        }

        job->target()->set_successful( errors == 0 );
    }
    else
    {
        build_tool_->error( "%s", job->target()->failed_dependencies().c_str() );
        job->target()->set_successful( false );
        job->set_state( JOB_COMPLETE );
    }    
}

void Scheduler::execute_finished( int exit_code, Context* context, process::Environment* environment )
{
    SWEET_ASSERT( context );

    process_begin( context );
    context->context_thread().resume()
        ( exit_code )
    .end();
    process_end( context );

    // The environment is deleted here for symmetry with its construction in 
    // the main thread in the Lua bindings along with filters and arguments.
    delete environment;
}

void Scheduler::filter_finished( Filter* filter, Arguments* arguments )
{
    // Delete filters and arguments on the main thread to avoid accessing the
    // Lua virtual machine from multiple threads as happens if the filter or 
    // arguments are deleted in the worker threads provided by the Executor.  
    delete filter;
    delete arguments;
}

void Scheduler::buildfile_finished( Context* context, bool success )
{
    SWEET_ASSERT( context );
    if ( context->context_thread().get_state() == LUA_THREAD_SUSPENDED )
    {
        process_begin( context );
        context->context_thread().resume()
            ( success ? 0 : 1 )
        .end();
        process_end( context );    
    }
}

void Scheduler::output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory )
{
    SWEET_ASSERT( build_tool_ );
    if ( filter )
    {
        Context* context = allocate_context( working_directory );
        process_begin( context );
        lua::AddParameter add_parameter = context->context_thread().call( LUA_REGISTRYINDEX, filter->reference() );
        add_parameter( output );
        if ( arguments )
        {
            arguments->push_arguments( add_parameter );
        }
        add_parameter.end();                
        process_end( context );
    }
    else
    {    
        build_tool_->output( output.c_str() );
    }
}

void Scheduler::error( const std::string& what, Context* context )
{
    SWEET_ASSERT( build_tool_ );
    build_tool_->error( what.c_str() );
    if ( context )
    {
        SWEET_ASSERT( !active_contexts_.empty() );    
        SWEET_ASSERT( context == active_contexts_.back() || find(active_contexts_.begin(), active_contexts_.end(), context) == active_contexts_.end() );
        if ( context == active_contexts_.back() )
        {
            active_contexts_.pop_back();
        }
        destroy_context( context );
    }
}

void Scheduler::push_output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::output, this, output, filter, arguments, working_directory) );
    results_condition_.notify_all();
}

void Scheduler::push_error( const std::exception& exception, Context* context )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::error, this, string(exception.what()), context) );
    results_condition_.notify_all();
}

void Scheduler::push_execute_finished( int exit_code, Context* context, process::Environment* environment )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    --jobs_;
    results_.push_back( std::bind(&Scheduler::execute_finished, this, exit_code, context, environment) );
    results_condition_.notify_all();
}

void Scheduler::push_filter_finished( Filter* filter, Arguments* arguments )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::filter_finished, this, filter, arguments) );
    results_condition_.notify_all();
}

void Scheduler::execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Context* context )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    build_tool_->executor()->execute( command, command_line, environment, dependencies_filter, stdout_filter, stderr_filter, arguments, context );
    ++jobs_;
}

void Scheduler::wait()
{
    while ( dispatch_results() )
    {
    }
}

int Scheduler::postorder( int function, Target* target )
{
    struct ScopedVisit
    {
        Target* target_;

        ScopedVisit( Target* target )
        : target_( target )
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( !target_->visiting() );
            target_->set_visited( true );
            target_->set_visiting( true );
        }

        ~ScopedVisit()
        {
            SWEET_ASSERT( target_->visiting() );
            target_->set_visiting( false );
        }
    };

    struct Postorder
    {
        BuildTool* build_tool_;
        list<Job> jobs_;
        int failures_;
        
        Postorder( BuildTool* build_tool )
        : build_tool_( build_tool ),
          jobs_(),
          failures_( 0 )
        {
            SWEET_ASSERT( build_tool_ );
            build_tool_->graph()->begin_traversal();
        }
        
        ~Postorder()
        {
            build_tool_->graph()->end_traversal();
        }
    
        void remove_complete_jobs()
        {
            list<Job>::iterator job = jobs_.begin();
            while ( job != jobs_.end() )
            {
                if ( job->state() == JOB_COMPLETE )
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

            while ( job != jobs_.end() && (job->state() != JOB_WAITING || job->height() > height) )
            {
                height = std::min( height, job->height() );
                ++job;
            }

            if ( job != jobs_.end() )
            {
                SWEET_ASSERT( job->state() == JOB_WAITING );
                job->set_state( JOB_PROCESSING );
            }

            return job != jobs_.end() ? &(*job) : NULL;
        }
        
        bool empty() const
        {
            return jobs_.empty();
        }

        int failures() const
        {
            return failures_;
        }

        void visit( Target* target )
        {
            SWEET_ASSERT( target );

            if ( !target->visited() )
            {
                ScopedVisit visit( target );

                int height = 0;
                int i = 0;
                Target* dependency = target->any_dependency( i );
                while ( dependency )
                {
                    if ( !dependency->visiting() )
                    {
                        Postorder::visit( dependency );
                        height = std::max( height, dependency->postorder_height() + 1 );
                    }
                    else
                    {
                        build_tool_->error( "Cyclic dependency from %s to %s in postorder traversal", target->error_identifier().c_str(), dependency->error_identifier().c_str() );
                        dependency->set_successful( true );
                        ++failures_;
                    }

                    ++i;
                    dependency = target->any_dependency( i );
                }

                if ( target->referenced_by_script() && target->working_directory() )
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

    Graph* graph = build_tool_->graph();
    if ( graph->traversal_in_progress() )
    {
        SWEET_ERROR( PostorderCalledRecursivelyError("Postorder called from within another bind or postorder traversal") );
        return 0;
    }
    
    Postorder postorder( build_tool_ );
    postorder.visit( target ? target : graph->root_target() );
    failures_ = postorder.failures();
    if ( failures_ == 0 )
    {
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
    }
    return failures_;
}

Context* Scheduler::context() const
{
    return !active_contexts_.empty() ? active_contexts_.back() : NULL;
}

Context* Scheduler::allocate_context( Target* working_directory, Job* job )
{
    SWEET_ASSERT( working_directory );
    SWEET_ASSERT( !job || job->working_directory() == working_directory );    
    Context* context = new Context( boost::filesystem::path(""), build_tool_ );
    context->reset_directory_to_target( working_directory );
    context->set_job( job );
    return context;
}

void Scheduler::free_context( Context* context )
{
    SWEET_ASSERT( context );

    Job* job = context->job();
    if ( job )
    {
        job->set_state( JOB_COMPLETE );
    }

    delete context;
}

void Scheduler::destroy_context( Context* context )
{
    SWEET_ASSERT( context );

    Job* job = context->job();
    if ( job )
    {
        job->set_state( JOB_COMPLETE );
        job->target()->set_successful( false );
    }

    delete context;
}

bool Scheduler::dispatch_results()
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
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

void Scheduler::process_begin( Context* context )
{
    SWEET_ASSERT( context );
    active_contexts_.push_back( context );
    build_tool_->error_policy().push_errors();
}

int Scheduler::process_end( Context* context )
{
    SWEET_ASSERT( context );
    SWEET_ASSERT( !active_contexts_.empty() );
    SWEET_ASSERT( active_contexts_.back() == context );

    active_contexts_.pop_back();
    int errors = build_tool_->error_policy().pop_errors();
    LuaThreadState state = context->context_thread().get_state();
    if ( errors == 0 && state != lua::LUA_THREAD_ERROR )
    {
        if ( state == lua::LUA_THREAD_READY )
        {
            context->context_thread().fire_returned();
            free_context( context );
        }
    }
    else
    {
        context->context_thread().fire_errored();
        destroy_context( context );
    }
    return errors;
}

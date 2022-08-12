//
// Scheduler.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Scheduler.hpp"
#include "Target.hpp"
#include "Graph.hpp"
#include "Forge.hpp"
#include "Job.hpp"
#include "Context.hpp"
#include "Executor.hpp"
#include "Reader.hpp"
#include "Filter.hpp"
#include "Arguments.hpp"
#include <process/Environment.hpp>
#include <luaxx/luaxx.hpp>
#include <error/ErrorPolicy.hpp>
#include <list>
#include <string>
#include <memory>
#include <algorithm>
#include <lua.hpp>

using std::sort;
using std::list;
using std::vector;
using std::string;
using std::unique_ptr;
using std::function;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::luaxx;
using namespace sweet::forge;

Scheduler::Scheduler( Forge* forge )
: forge_( forge )
, active_contexts_()
, results_mutex_()
, results_condition_()
, results_()
, pending_results_( 0 )
, buildfile_calls_( 0 )
{
    SWEET_ASSERT( forge_ );
}

void Scheduler::load( const boost::filesystem::path& path )
{
    SWEET_ASSERT( path.is_absolute() );
    Context* context = allocate_context( forge_->graph()->target(path.parent_path().generic_string()) );
    process_begin( context );
    lua_State* lua_state = context->lua_state();
    dofile( lua_state, path.string().c_str() );
    process_end( context );
    wait();
}

void Scheduler::script( const boost::filesystem::path& working_directory, const std::string& script )
{
    if ( !script.empty() )
    {
        Context* context = allocate_context( forge_->graph()->target(working_directory.generic_string()) );
        process_begin( context );
        lua_State* lua_state = context->lua_state();
        doscript( lua_state, script.c_str() );
        process_end( context );
        wait();
    }
}

void Scheduler::command( const boost::filesystem::path& working_directory, const std::string& function )
{    
    if ( !function.empty() )
    {
        Context* context = allocate_context( forge_->graph()->target(working_directory.generic_string()) );
        process_begin( context );
        lua_State* lua_state = context->lua_state();
        SWEET_ASSERT( lua_state );
        lua_getglobal( lua_state, function.c_str() );
        resume( lua_state, 0 );
        process_end( context );
    }
    wait();
    SWEET_ASSERT( buildfile_calls_ == 0 );
}

int Scheduler::buildfile( const boost::filesystem::path& path )
{
    SWEET_ASSERT( path.is_absolute() );
    SWEET_ASSERT( !active_contexts_.empty() );

    Target* buildfile = forge_->graph()->target( path.generic_string() );
    Target* working_directory = buildfile->parent();
    SWEET_ASSERT( forge_->graph()->target(path.parent_path().generic_string()) == working_directory );

    Context* calling_context = active_contexts_.back();
    Context* context = allocate_context( working_directory );
    context->set_current_buildfile( buildfile );
    context->set_buildfile_calling_context( calling_context );
    process_begin( context );

    lua_State* lua_state = context->lua_state();
    SWEET_ASSERT( lua_state );
    dofile( lua_state, path.string().c_str() );
    bool yielded = lua_status( lua_state ) == LUA_YIELD;
    int errors = process_end( context );
    buildfile_calls_ += yielded ? 1 : 0;
    return yielded ? -1 : errors;
}

void Scheduler::preorder_visit( int function, Job* job )
{
    SWEET_ASSERT( job );

    Target* target = job->target();
    SWEET_ASSERT( target );
    SWEET_ASSERT( !target->visiting() );
    target->set_visited( true );
    target->set_visiting( true );

    Context* context = allocate_context( target->working_directory(), job );
    process_begin( context );

    lua_State* lua_state = context->lua_state();
    lua_rawgeti( lua_state, LUA_REGISTRYINDEX, function );
    luaxx_push( lua_state, target );
    resume( lua_state, 1 );
    
    int errors = process_end( context );
    if ( errors > 0 )
    {
        forge_->errorf( "Preorder visit of '%s' failed", target->id().c_str() );
    }
    target->set_successful( errors == 0 );
}

void Scheduler::postorder_visit( int function, Job* job )
{
    SWEET_ASSERT( job );

    Target* target = job->target();
    SWEET_ASSERT( target );

    if ( target->buildable() )
    {
        Context* context = allocate_context( job->working_directory(), job );
        process_begin( context );

        lua_State* lua_state = context->lua_state();
        lua_rawgeti( lua_state, LUA_REGISTRYINDEX, function );
        luaxx_push( lua_state, target );
        resume( lua_state, 1 );
        
        int errors = process_end( context );
        if ( errors > 0 )
        {
            forge_->errorf( "Postorder visit of '%s' failed", target->id().c_str() );
        }
        job->target()->set_successful( errors == 0 );
    }
    else
    {
        forge_->error( target->failed_dependencies().c_str() );
        target->set_successful( false );
        job->set_state( JOB_COMPLETE );
    }    
}

void Scheduler::execute_finished( int exit_code, Context* context, process::Environment* environment )
{
    SWEET_ASSERT( context );

    process_begin( context );
    lua_State* lua_state = context->lua_state();
    lua_pushinteger( lua_state, exit_code );
    resume( lua_state, 1 );
    process_end( context );

    // The environment is deleted here for symmetry with its construction in 
    // the main thread in the Lua bindings along with filters and arguments.
    delete environment;
}

void Scheduler::read_finished( Filter* filter, Arguments* arguments )
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
    if ( lua_status(context->lua_state()) == LUA_YIELD )
    {
        process_begin( context );
        lua_State* lua_state = context->lua_state();
        lua_pushinteger( lua_state, success ? 0 : 1 );
        resume( lua_state, 1 );
        process_end( context );
        --buildfile_calls_;
    }
}

void Scheduler::output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory )
{
    SWEET_ASSERT( forge_ );
    if ( filter )
    {
        Context* context = allocate_context( working_directory );
        process_begin( context );
        lua_State* lua_state = context->lua_state();
        lua_rawgeti( lua_state, LUA_REGISTRYINDEX, filter->reference() );
        lua_pushlstring( lua_state, output.c_str(), output.size() );
        int parameters = 1;
        if ( arguments )
        {
            parameters += arguments->push_arguments( lua_state );
        }
        resume( lua_state, parameters );
        process_end( context );
    }
    else
    {    
        forge_->output( output.c_str() );
    }
}

void Scheduler::error( const std::string& what )
{
    SWEET_ASSERT( forge_ );
    forge_->error( what.c_str() );
}

void Scheduler::push_output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::output, this, output, filter, arguments, working_directory) );
    results_condition_.notify_all();
}

void Scheduler::push_errorf( const char* format, ... )
{
    char message [1024];
    va_list args;
    va_start( args, format );
    vsnprintf( message, sizeof(message), format, args );
    va_end( args );
    message[sizeof(message) - 1] = 0;
    std::unique_lock<std::mutex> lock( results_mutex_ );
    results_.push_back( std::bind(&Scheduler::error, this, string(message)) );
    results_condition_.notify_all();
}

void Scheduler::push_execute_finished( int exit_code, Context* context, process::Environment* environment )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    SWEET_ASSERT( pending_results_ > 0 );
    --pending_results_;
    results_.push_back( std::bind(&Scheduler::execute_finished, this, exit_code, context, environment) );
    results_condition_.notify_all();
}

void Scheduler::push_read_finished( Filter* filter, Arguments* arguments )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    SWEET_ASSERT( pending_results_ > 0 );
    --pending_results_;
    results_.push_back( std::bind(&Scheduler::read_finished, this, filter, arguments) );
    results_condition_.notify_all();
}

void Scheduler::execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Context* context )
{
    SWEET_ASSERT( !command.empty() );
    std::unique_lock<std::mutex> lock( results_mutex_ );
    forge_->executor()->execute( command, command_line, environment, dependencies_filter, stdout_filter, stderr_filter, arguments, context );
    ++pending_results_;
}

void Scheduler::read( intptr_t fd_or_handle, Filter* filter, Arguments* arguments, Target* working_directory )
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    forge_->reader()->read( fd_or_handle, filter, arguments, working_directory );
    ++pending_results_;
}

void Scheduler::prune()
{
    if ( !active_contexts_.empty() )
    {
        Context* context = active_contexts_.back();
        SWEET_ASSERT( context );
        context->set_prune( true );
    }
    else
    {
        forge_->error( "prune called outside preorder" );
    }
}

void Scheduler::wait()
{
    while ( dispatch_results() )
    {
    }
}

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

int Scheduler::preorder( Target* target, int function )
{
    struct Preorder
    {
        Forge* forge_;
        list<Job> jobs_;
        
        Preorder( Forge* forge )
        : forge_( forge )
        , jobs_()
        {
            SWEET_ASSERT( forge_ );
            forge_->graph()->begin_traversal();
        }
        
        ~Preorder()
        {
            forge_->graph()->end_traversal();
        }

        void begin_traversal( Target* target )
        {
            SWEET_ASSERT( target );
            SWEET_ASSERT( !target->visited() );
            if ( target->referenced_by_script() && target->working_directory() )
            {
                jobs_.push_back( Job(target) );
            }
        }

        Job* pull_job()
        {
            remove_complete_jobs();

            list<Job>::iterator job = jobs_.begin();
            while ( job != jobs_.end() && job->state() != JOB_WAITING )
            {
                ++job;
            }

            if ( job != jobs_.end() )
            {
                SWEET_ASSERT( job->state() == JOB_WAITING );
                job->set_state( JOB_PROCESSING );
            }

            return job != jobs_.end() ? &(*job) : nullptr;
        }
        
        void remove_complete_jobs()
        {
            list<Job>::iterator job = jobs_.begin();
            list<Job>::iterator end = jobs_.end();
            while ( job != end )
            {
                if ( job->state() == JOB_COMPLETE )
                {
                    Target* target = job->target();
                    SWEET_ASSERT( target );
                    SWEET_ASSERT( target->visiting() );

                    if ( target->successful() && !job->prune() )
                    {
                        int i = 0;
                        Target* dependency = target->any_dependency( i );
                        while ( dependency )
                        {
                            if ( dependency->referenced_by_script() && dependency->working_directory() && !dependency->visited() )
                            {
                                if ( !dependency->visiting() )
                                {
                                    jobs_.push_back( Job(dependency) );
                                }
                                else
                                {
                                    forge_->errorf( "Cyclic dependency from %s to %s in preorder", target->error_identifier().c_str(), dependency->error_identifier().c_str() );
                                    dependency->set_successful( true );
                                }
                            }
                            ++i;
                            dependency = target->any_dependency( i );
                        }
                    }

                    target->set_visiting( false );
                    job = jobs_.erase( job );
                }
                else
                {
                    ++job;
                }
            }
        }

        bool empty() const
        {
            return jobs_.empty();
        }
    };

    Graph* graph = forge_->graph();
    if ( graph->traversal_in_progress() )
    {
        forge_->error( "Preorder called from within preorder or postorder" );
        return 1;
    }

    error::ErrorPolicy& error_policy = forge_->error_policy();
    error_policy.push_errors();

    Preorder preorder( forge_ );
    preorder.begin_traversal( target ? target : graph->root_target() );
    while ( !preorder.empty() )
    {
        Job* job = preorder.pull_job();
        while ( job )
        {
            preorder_visit( function, job );
            job = preorder.pull_job();
        }
        dispatch_results();
    }
    wait();

    return error_policy.pop_errors();
}

int Scheduler::postorder( Target* target, int function )
{
    struct Postorder
    {
        Forge* forge_;
        list<Job> jobs_;
        
        Postorder( Forge* forge )
        : forge_( forge )
        , jobs_()
        {
            SWEET_ASSERT( forge_ );
            forge_->graph()->begin_traversal();
        }
        
        ~Postorder()
        {
            forge_->graph()->end_traversal();
        }
    
        Job* pull_job()
        {
            remove_complete_jobs();

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

        bool empty() const
        {
            return jobs_.empty();
        }

        void visit( Target* target )
        {
            SWEET_ASSERT( target );

            if ( !target->visited() )
            {
                ScopedVisit visit( target );

                int height = 0;
                int i = 0;
                Target* dependency = target->binding_dependency( i );
                while ( dependency )
                {
                    if ( !dependency->visiting() )
                    {
                        Postorder::visit( dependency );
                        height = std::max( height, dependency->postorder_height() + 1 );
                    }
                    else
                    {
                        forge_->errorf( "Cyclic dependency from %s to %s in postorder", target->error_identifier().c_str(), dependency->error_identifier().c_str() );
                        dependency->set_successful( true );
                    }

                    ++i;
                    dependency = target->binding_dependency( i );
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

    Graph* graph = forge_->graph();
    if ( graph->traversal_in_progress() )
    {
        forge_->errorf( "Postorder called from within preorder or postorder" );
        return 1;
    }
    
    error::ErrorPolicy& error_policy = forge_->error_policy();
    error_policy.push_errors();

    Postorder postorder( forge_ );
    postorder.visit( target ? target : graph->root_target() );
    if ( error_policy.errors() == 0 )
    {
        while ( !postorder.empty() )
        {
            Job* job = postorder.pull_job();
            while ( job )
            {
                postorder_visit( function, job );
                job = postorder.pull_job();
            }
            dispatch_results();
        }
        wait();
    }

    return error_policy.pop_errors();
}

Context* Scheduler::context() const
{
    return !active_contexts_.empty() ? active_contexts_.back() : NULL;
}

Context* Scheduler::allocate_context( Target* working_directory, Job* job )
{
    SWEET_ASSERT( working_directory );
    SWEET_ASSERT( !job || job->working_directory() == working_directory );    
    Context* context = new Context( forge_ );
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
        job->set_prune( context->prune() );
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
        job->set_prune( true );
        job->target()->set_successful( false );
    }

    delete context;
}

bool Scheduler::dispatch_results()
{
    std::unique_lock<std::mutex> lock( results_mutex_ );
    while ( results_.empty() && pending_results_ > 0 )
    {
        results_condition_.wait( lock );
    }

    while ( !results_.empty() )
    {
        function<void()> result = move( results_.front() );
        results_.pop_front();
        lock.unlock();
        result();
        lock.lock();
    }
    
    return pending_results_ > 0;
}

void Scheduler::process_begin( Context* context )
{
    SWEET_ASSERT( context );
    context->set_prune( false );
    active_contexts_.push_back( context );
    forge_->error_policy().push_errors();
}

int Scheduler::process_end( Context* context )
{
    SWEET_ASSERT( context );
    SWEET_ASSERT( !active_contexts_.empty() );
    SWEET_ASSERT( active_contexts_.back() == context );

    active_contexts_.pop_back();
    int errors = forge_->error_policy().pop_errors();
    lua_State* lua_state = context->lua_state();
    if ( lua_status(lua_state) != LUA_YIELD )
    {
        bool successful = errors == 0 && lua_status( lua_state ) == LUA_OK;
        Context* buildfile_calling_context = context->buildfile_calling_context();
        if ( buildfile_calling_context )
        {
            buildfile_finished( buildfile_calling_context, successful );
        }
        if ( successful )
        {
            free_context( context );
        }
        else
        {
            destroy_context( context );
        }
    }
    return errors;
}

void Scheduler::dofile( lua_State* lua_state, const char* filename )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( filename );
    int result = luaL_loadfile( lua_state, filename );
    switch ( result )
    {
        case LUA_OK:
        {
            resume( lua_state, 0 );
            break;
        }

        case LUA_ERRSYNTAX:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "%s", lua_tolstring(lua_state, -1, nullptr) );
            break;
        }

        case LUA_ERRMEM:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Out of memory loading '%s'", filename );
            break;
        }

        case LUA_ERRFILE:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "File not found loading '%s'", filename );
            break;
        }

        default:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Unexpected error loading '%s'", filename );
            break;
        }
    }
}

void Scheduler::doscript( lua_State* lua_state, const char* script )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( script );
    int result = luaL_loadstring( lua_state, script );
    switch ( result )
    {
        case LUA_OK:
        {
            resume( lua_state, 0 );
            break;
        }

        case LUA_ERRSYNTAX:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "%s", lua_tolstring(lua_state, -1, nullptr) );
            break;
        }

        case LUA_ERRMEM:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Out of memory loading script" );
            break;
        }

        case LUA_ERRFILE:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "File not found loading script" );
            break;
        }

        default:
        {
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Unexpected error loading script" );
            break;
        }
    }
}

void Scheduler::resume( lua_State* lua_state, int parameters )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( parameters >= 0 );

    int result = lua_resume( lua_state, nullptr, parameters );
    switch ( result )
    {
        case 0:
            break;

        case LUA_YIELD:
            break;            

        case LUA_ERRRUN:
        {
            char message [1024];
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "%s", luaxx_stack_trace_for_resume(lua_state, forge_->stack_trace_enabled(), message, sizeof(message)) );
            break;
        }

        case LUA_ERRMEM:
        {
            char message [1024];
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Out of memory - %s", luaxx_stack_trace_for_resume(lua_state, forge_->stack_trace_enabled(), message, sizeof(message)) );
            break;
        }

        case LUA_ERRERR:
        {
            char message [1024];
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Error handler failed - %s", luaxx_stack_trace_for_resume(lua_state, forge_->stack_trace_enabled(), message, sizeof(message)) );
            break;
        }
        
        case -1:
        {
            char message [1024];
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Execution failed due to an unhandled C++ exception - %s", luaxx_stack_trace_for_resume(lua_state, forge_->stack_trace_enabled(), message, sizeof(message)) );
            break;
        }

        default:
        {
            SWEET_ASSERT( false );
            char message [1024];
            error::ErrorPolicy* error_policy = &forge_->error_policy();
            error_policy->error( true, "Execution failed in an unexpected way - %s", luaxx_stack_trace_for_resume(lua_state, forge_->stack_trace_enabled(), message, sizeof(message)) );
            break;
        }
    }    
}

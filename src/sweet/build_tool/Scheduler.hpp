#ifndef SWEET_BUILD_TOOL_SCHEDULER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_SCHEDULER_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/thread/Condition.hpp>
#include <sweet/thread/Mutex.hpp>
#include <sweet/path/Path.hpp>
#include <deque>
#include <vector>
#include <functional>

namespace sweet
{

namespace process
{

class Environment;

}

namespace lua
{

class LuaValue;

}

namespace build_tool
{

class Job;
class Context;
class Arguments;
class Pattern;
class Target;
class BuildTool;

/**
// Handle general processing and calls into Lua from loading buildfiles,
// traversals, match callbacks from scanning source files and executing
// external processes.
*/
class Scheduler
{
    BuildTool* build_tool_; ///< The BuildTool that this Scheduler is part of.
    std::vector<Context*> contexts_; ///< The currently allocated Environments.
    std::vector<Context*> free_contexts_; ///< The Environments that are free and waiting to be assigned a Job.
    std::vector<Context*> active_contexts_; ///< The stack of Environments that are currently executing Lua scripts.
    thread::Mutex results_mutex_; ///< The mutex that ensures exclusive access to the results queue.
    thread::Condition results_condition_; ///< The Condition that is used to wait for results.
    std::deque<std::function<void()> > results_; ///< The functions to be executed as a result of jobs processing in the thread pool.
    int jobs_; ///< The number of jobs that are running.
    int failures_; ///< The number of failures in the most recent preorder or postorder traversal.

    public:
        Scheduler( BuildTool* build_tool );
        ~Scheduler();

        void load( const path::Path& path );
        void command( const path::Path& path, const std::string& command );
        void execute( const path::Path& path, const std::string& function );
        void execute( const char* start, const char* finish );        
        void buildfile( const path::Path& path );
        void call( const path::Path& path, const std::string& function );
        void postorder_visit( const lua::LuaValue& function, Job* job );
        void execute_finished( int exit_code, Context* context, process::Environment* environment );
        void filter_finished( lua::LuaValue* filter, Arguments* arguments );
        void output( const std::string& output, lua::LuaValue* filter, Arguments* arguments, Target* working_directory );
        void error( const std::string& what, Context* context );

        void push_output( const std::string& output, lua::LuaValue* filter, Arguments* arguments, Target* working_directory );
        void push_error( const std::exception& exception, Context* context );
        void push_execute_finished( int exit_code, Context* context, process::Environment* environment );
        void push_filter_finished( lua::LuaValue* filter, Arguments* arguments );
        void push_scan_finished( Arguments* arguments );

        void execute( const std::string& command, const std::string& command_line, process::Environment* environment, lua::LuaValue* dependencies_filter, lua::LuaValue* stdout_filter, lua::LuaValue* stderr_filter, Arguments* arguments, Context* context );
        void wait();
        
        int postorder( const lua::LuaValue& function, Target* target );        

        Context* context() const;

    private:
        bool dispatch_results();
        void process_begin( Context* context );
        int process_end( Context* context );
        Context* allocate_context( Target* working_directory, Job* job = NULL );
        void free_context( Context* context );
        void destroy_context( Context* context );
        void push_context( Context* context );
        int pop_context( Context* context );
};

}

}

#endif

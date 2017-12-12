#ifndef SWEET_BUILD_TOOL_SCHEDULER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_SCHEDULER_HPP_INCLUDED

#include <boost/filesystem/path.hpp>
#include <deque>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace sweet
{

namespace process
{

class Environment;

}

namespace build_tool
{

class Job;
class Context;
class Arguments;
class Filter;
class Target;
class BuildTool;
class BuildfileEventSink;

/**
// Handle general processing and calls into Lua from loading buildfiles,
// traversals, match callbacks from scanning source files and executing
// external processes.
*/
class Scheduler
{
    BuildTool* build_tool_; ///< The BuildTool that this Scheduler is part of.
    BuildfileEventSink* buildfile_event_sink_; ///< The LuaThreadEventSink that handles `buildfile()` coroutines finishing.
    std::vector<Context*> contexts_; ///< The currently allocated Contexts.
    std::vector<Context*> free_contexts_; ///< The Contexts that are free and waiting to be assigned a Job.
    std::vector<Context*> active_contexts_; ///< The stack of Contexts that are currently executing Lua scripts.
    std::mutex results_mutex_; ///< The mutex that ensures exclusive access to the results queue.
    std::condition_variable results_condition_; ///< The Condition that is used to wait for results.
    std::deque<std::function<void()> > results_; ///< The functions to be executed as a result of jobs processing in the thread pool.
    int jobs_; ///< The number of jobs that are running.
    int failures_; ///< The number of failures in the most recent preorder or postorder traversal.

    public:
        Scheduler( BuildTool* build_tool );
        ~Scheduler();

        void load( const boost::filesystem::path& path );
        void command( const boost::filesystem::path& path, const std::string& command );
        void execute( const boost::filesystem::path& path, const std::string& function );
        void execute( const char* start, const char* finish );        
        int buildfile( const boost::filesystem::path& path );
        void call( const boost::filesystem::path& path, const std::string& function );
        void postorder_visit( int function, Job* job );
        void execute_finished( int exit_code, Context* context, process::Environment* environment );
        void filter_finished( Filter* filter, Arguments* arguments );
        void buildfile_finished( Context* context, bool success );
        void output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory );
        void error( const std::string& what, Context* context );

        void push_output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory );
        void push_error( const std::exception& exception, Context* context );
        void push_execute_finished( int exit_code, Context* context, process::Environment* environment );
        void push_filter_finished( Filter* filter, Arguments* arguments );

        void execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Context* context );
        void wait();
        
        int postorder( int function, Target* target );        

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

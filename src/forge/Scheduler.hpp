#ifndef FORGE_SCHEDULER_HPP_INCLUDED
#define FORGE_SCHEDULER_HPP_INCLUDED

#include <boost/filesystem/path.hpp>
#include <deque>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

struct lua_State;

namespace sweet
{

namespace process
{

class Environment;

}

namespace forge
{

class Job;
class Context;
class Arguments;
class Filter;
class Target;
class Forge;

/**
// Handle general processing and calls into Lua from loading buildfiles,
// traversals, match callbacks from scanning source files and executing
// external processes.
*/
class Scheduler
{
    Forge* forge_; ///< The Forge that this Scheduler is part of.
    std::vector<Context*> active_contexts_; ///< The stack of Contexts that are currently executing Lua scripts.
    std::mutex results_mutex_; ///< The mutex that ensures exclusive access to the results queue.
    std::condition_variable results_condition_; ///< The Condition that is used to wait for results.
    std::deque<std::function<void()> > results_; ///< The functions to be executed as a result of jobs processing in the thread pool.
    std::vector<Target*> buildfiles_stack_; ///< The stack of currently processing buildfiles.
    int pending_results_; ///< The number of results waiting on execute and read tasks to finish.
    int buildfile_calls_; ///< The number of outstanding calls made to load buildfiles.

    public:
        Scheduler( Forge* forge );

        void load( const boost::filesystem::path& path );
        void script( const boost::filesystem::path& working_directory, const std::string& script );
        void command( const boost::filesystem::path& working_directory, const std::string& command );
        int buildfile( const boost::filesystem::path& path );
        void preorder_visit( int function, Job* job );
        void postorder_visit( int function, Job* job );
        void execute_finished( int exit_code, Context* context, process::Environment* environment );
        void read_finished( Filter* filter, Arguments* arguments );
        void buildfile_finished( Context* context, bool success );
        void output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory );
        void error( const std::string& what );

        void push_output( const std::string& output, Filter* filter, Arguments* arguments, Target* working_directory );
        void push_errorf( const char* format, ... );
        void push_execute_finished( int exit_code, Context* context, process::Environment* environment );
        void push_read_finished( Filter* filter, Arguments* arguments );

        void execute( const std::string& command, const std::string& command_line, process::Environment* environment, Filter* dependencies_filter, Filter* stdout_filter, Filter* stderr_filter, Arguments* arguments, Context* context );
        void read( intptr_t fd_or_handle, Filter* filter, Arguments* arguments, Target* working_directory );
        void prune();
        void wait();
        
        int preorder( Target* target, int function );        
        int postorder( Target* target, int function );        

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
        void dofile( lua_State* lua_state, const char* filename );
        void doscript( lua_State* lua_state, const char* script );
        void resume( lua_State* lua_state, int parameters );
};

}

}

#endif

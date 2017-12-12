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

namespace lua
{

class LuaValue;

}

namespace build_tool
{

class Job;
class Environment;
class Scanner;
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
    std::vector<Environment*> environments_; ///< The currently allocated Environments.
    std::vector<Environment*> free_environments_; ///< The Environments that are free and waiting to be assigned a Job.
    std::vector<Environment*> active_environments_; ///< The stack of Environments that are currently executing Lua scripts.
    thread::Mutex results_mutex_; ///< The mutex that ensures exclusive access to the results queue.
    thread::Condition results_condition_; ///< The Condition that is used to wait for results.
    std::deque<std::function<void()> > results_; ///< The functions to be executed as a result of jobs processing in the thread pool.
    int jobs_; ///< The number of jobs that are running.
    int failures_; ///< The number of failures in the most recent preorder or postorder traversal.
    bool traversing_; ///< True if a preorder or postorder traversal is happening otherwise false.

    public:
        Scheduler( BuildTool* build_tool );
        ~Scheduler();

        void set_traversing( bool traversing );
        bool is_traversing() const;

        void load( const path::Path& path );
        void command( const path::Path& path, const std::string& command );
        void execute( const path::Path& path, const std::string& function );
        void execute( const char* start, const char* finish );        
        void buildfile( const path::Path& path );
        void call( const path::Path& path, const std::string& function );
        void preorder_visit( const lua::LuaValue& function, Target* target );
        void postorder_visit( const lua::LuaValue& function, Job* job );
        void execute_finished( int exit_code, Environment* environment, Arguments* arguments );
        void scan_finished( Arguments* arguments );
        void output( const std::string& output, Scanner* scanner, Arguments* arguments, Target* working_directory );
        void match( const Pattern* pattern, Target* target, const std::string& match, Arguments* arguments, Target* working_directory );
        void error( const std::string& what, Environment* environment );

        void push_output( const std::string& output, Scanner* scanner, Arguments* arguments, Target* working_directory );
        void push_error( const std::exception& exception, Environment* environment );
        void push_match( const Pattern* pattern, const std::string& match, Arguments* arguments, Target* working_directory, Target* target );
        void push_execute_finished( int exit_code, Environment* environment, Arguments* arguments );
        void push_scan_finished( Arguments* arguments );

        void execute( const std::string& command, const std::string& command_line, Scanner* scanner, Arguments* arguments, Environment* environment );
        void scan( Target* target, Scanner* scanner, Arguments* arguments, Target* working_directory, Environment* environment );
        void wait();
        
        int preorder( const lua::LuaValue& function, Target* target );
        int postorder( const lua::LuaValue& function, Target* target );        

        Environment* environment() const;

    private:
        bool dispatch_results();
        void process_begin( Environment* environment );
        int process_end( Environment* environment );
        Environment* allocate_environment( Target* working_directory, Job* job = NULL );
        void free_environment( Environment* environment );
        void destroy_environment( Environment* environment );
        void push_environment( Environment* environment );
        int pop_environment( Environment* environment );
};

}

}

#endif

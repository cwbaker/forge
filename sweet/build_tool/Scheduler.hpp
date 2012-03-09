//
// Scheduler.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_SCHEDULER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_SCHEDULER_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/pointer/ptr.hpp>
#include <sweet/thread/Condition.hpp>
#include <sweet/thread/Mutex.hpp>
#include <sweet/path/Path.hpp>
#include <boost/function.hpp>
#include <deque>
#include <vector>

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
    std::vector<ptr<Environment>> free_environments_; ///< The Environments that are free and waiting to be assigned a Job.
    thread::Mutex results_mutex_; ///< The mutex that ensures exclusive access to the results queue.
    thread::Condition results_condition_; ///< The Condition that is used to wait for results.
    std::deque<boost::function<void()>> results_; ///< The functions to be executed as a result of jobs processing in the thread pool.
    int jobs_; ///< The number of jobs that are running.
    int failures_; ///< The number of failures in the most recent preorder or postorder traversal.
    bool traversing_; ///< True if a preorder or postorder traversal is happening otherwise false.

    public:
        Scheduler( BuildTool* build_tool );
        ~Scheduler();

        void set_traversing( bool traversing );
        bool is_traversing() const;

        void execute( const path::Path& path );
        void execute( const char* start, const char* finish );
        void buildfile( const path::Path& path );        
        void preorder_visit( const lua::LuaValue& function, ptr<Target> target );
        void postorder_visit( const lua::LuaValue& function, Job* job );
        void execute_finished( int exit_code, ptr<Environment> environment );
        void scan_finished();
        void output( const std::string& output, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory );
        void match( const Pattern* pattern, ptr<Target> target, const std::string& match, ptr<Arguments> arguments, ptr<Target> working_directory );
        void error( const std::string& what, ptr<Environment> environment );

        void push_output( const std::string& output, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory );
        void push_error( const std::exception& exception, ptr<Environment> environment );
        void push_match( const Pattern* pattern, const std::string& match, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Target> target );
        void push_execute_finished( int exit_code, ptr<Environment> environment );
        void push_scan_finished();

        void execute( const std::string& command, const std::string& command_line, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Environment> environment );
        void scan( ptr<Target> target, ptr<Scanner> scanner, ptr<Arguments> arguments, ptr<Target> working_directory, ptr<Environment> environment );
        void wait();
        
        int preorder( const lua::LuaValue& function, ptr<Target> target );
        int postorder( const lua::LuaValue& function, ptr<Target> target );        

    private:
        bool dispatch_results();
        void process_begin( ptr<Environment> environment );
        void process_end( ptr<Environment> environment );
        ptr<Environment> allocate_environment( ptr<Target> working_directory, Job* job = NULL );
        void free_environment( ptr<Environment> environment );
        void destroy_environment( ptr<Environment> environment );
};

}

}

#endif

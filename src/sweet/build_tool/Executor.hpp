#ifndef SWEET_BUILD_TOOL_EXECUTOR_HPP_INCLUDED
#define SWEET_BUILD_TOOL_EXECUTOR_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/thread/Condition.hpp>
#include <sweet/thread/Mutex.hpp>
#include <vector>
#include <deque>
#include <functional>

namespace sweet
{

namespace thread
{

class Thread;

}

namespace process
{

class Process;

}

namespace build_tool
{

class Scanner;
class Arguments;
class Target;
class Environment;
class BuildTool;

/**
// A thread pool and queue of scan and execute calls to be executed in that
// thread pool.
*/
class Executor
{
    BuildTool* build_tool_; ///< The BuildTool that this Executor is part of.
    thread::Mutex jobs_mutex_; ///< The mutex that ensures exclusive access to this Executor.
    thread::Condition jobs_empty_condition_; ///< The condition attribute that is used to notify threads that there are jobs ready to be processed.
    thread::Condition jobs_ready_condition_; ///< The condition attribute that is used to notify threads that there are jobs ready to be processed.
    std::deque<std::function<void ()> > jobs_; ///< The functions to be executed in the thread pool.
    int maximum_parallel_jobs_; ///< The maximum number of parallel jobs to allow.
    std::vector<thread::Thread*> threads_; ///< The thread pool of Threads that process the Commands.
    bool done_; ///< Whether or not this Executor has finished processing (indicates to the threads in the thread pool that they should return).

    public:
        Executor( BuildTool* build_tool );
        ~Executor();
        void set_maximum_parallel_jobs( int maximum_parallel_jobs );
        int maximum_parallel_jobs() const;
        void execute( const std::string& command, const std::string& command_line, Scanner* scanner, Arguments* arguments, Environment* environment );
        void scan( Target* target, Scanner* scanner, Arguments* arguments, Target* working_directory, Environment* environment );

    private:
        static int thread_main( void* context );
        void thread_process();
        void thread_execute( process::Process* process, Scanner* scanner, Arguments* arguments, Target* working_directory, Environment* environment );
        void thread_scan( Target* target, Scanner* scanner, Arguments* arguments, Target* working_directory, Environment* environment );
        void start();
        void stop();
};

}

}

#endif

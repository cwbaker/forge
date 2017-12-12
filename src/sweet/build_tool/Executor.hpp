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

class Environment;
class Process;

}

namespace lua
{

class LuaValue;

}

namespace build_tool
{

class Arguments;
class Context;
class Target;
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
    std::string build_hooks_library_; ///< The full path to the build hooks library.
    int maximum_parallel_jobs_; ///< The maximum number of parallel jobs to allow.
    std::vector<thread::Thread*> threads_; ///< The thread pool of Threads that process the Commands.
    bool done_; ///< Whether or not this Executor has finished processing (indicates to the threads in the thread pool that they should return).

    public:
        Executor( BuildTool* build_tool );
        ~Executor();
        void set_build_hooks_library( const std::string& build_hook_library );
        const std::string& build_hooks_library() const;
        void set_maximum_parallel_jobs( int maximum_parallel_jobs );
        int maximum_parallel_jobs() const;
        void execute( const std::string& command, const std::string& command_line, process::Environment* environment, lua::LuaValue* dependencies_filter, lua::LuaValue* stdout_filter, lua::LuaValue* stderr_filter, Arguments* arguments, Context* context );

    private:
        static int thread_main( void* context );
        void thread_process();
        void thread_execute( const std::string& command, const std::string& command_line, process::Environment* environment, lua::LuaValue* dependencies_filter, lua::LuaValue* stdout_filter, lua::LuaValue* stderr_filter, Arguments* arguments, Target* working_directory, Context* context );
        void start();
        void stop();
        process::Environment* inject_build_hooks_macosx( process::Environment* environment, bool dependencies_filter_exists ) const;
        void inject_build_hooks_windows( process::Process* process, intptr_t write_dependencies_pipe ) const;
};

}

}

#endif

#ifndef SWEET_BUILD_TOOL_READER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_READER_HPP_INCLUDED

#include <sweet/thread/Thread.hpp>
#include <sweet/thread/Mutex.hpp>
#include <sweet/thread/Condition.hpp>
#include <vector>
#include <deque>
#include <functional>

namespace sweet
{

namespace thread
{

class Thread;

}

namespace lua
{

class LuaValue;

}

namespace build_tool
{

class Target;
class Arguments;
class BuildTool;

class Reader
{
    BuildTool* build_tool_; ///< The BuildTool that this Reader is part of.
    thread::Mutex jobs_mutex_; ///< The mutex that ensures exclusive access to this Reader's jobs.
    thread::Condition jobs_empty_condition_; ///< The condition attribute that notifies jobs are processed.
    thread::Condition jobs_ready_condition_; ///< The condition attribute that notifies jobs ready.
    std::deque<std::function<void ()> > jobs_; ///< The functions to be executed in the thread pool.
    std::vector<thread::Thread*> threads_; ///< The thread pool for this Reader.
    int active_jobs_; ///< The number of active jobs.
    bool done_; ///< Whether or not this Reader has finished processing.

public:
    Reader( BuildTool* build_tool );
    ~Reader();
    void read( intptr_t fd_or_handle, lua::LuaValue* filter, Arguments* arguments, Target* working_directory );

private:
    static int thread_main( void* context );
    void thread_process();
    void thread_read( intptr_t fd_or_handle, lua::LuaValue* filter, Arguments* arguments, Target* working_directory );
    void stop();
    size_t read( intptr_t fd_or_handle, void* buffer, size_t length ) const;
    void close( intptr_t fd_or_handle ) const;
};

}

}

#endif

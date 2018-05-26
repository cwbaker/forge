#ifndef SWEET_BUILD_TOOL_READER_HPP_INCLUDED
#define SWEET_BUILD_TOOL_READER_HPP_INCLUDED

#include <vector>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace sweet
{

namespace build_tool
{

class Target;
class Filter;
class Arguments;
class BuildTool;

class Reader
{
    BuildTool* build_tool_; ///< The BuildTool that this Reader is part of.
    std::mutex jobs_mutex_; ///< The mutex that ensures exclusive access to this Reader's jobs.
    std::condition_variable jobs_empty_condition_; ///< The condition attribute that notifies jobs are processed.
    std::condition_variable jobs_ready_condition_; ///< The condition attribute that notifies jobs ready.
    std::deque<std::function<void ()> > jobs_; ///< The functions to be executed in the thread pool.
    std::vector<std::thread*> threads_; ///< The thread pool for this Reader.
    int active_jobs_; ///< The number of active jobs.
    bool done_; ///< Whether or not this Reader has finished processing.

public:
    Reader( BuildTool* build_tool );
    ~Reader();
    int active_jobs() const;
    void read( intptr_t fd_or_handle, Filter* filter, Arguments* arguments, Target* working_directory );

private:
    static int thread_main( void* context );
    void thread_process();
    void thread_read( intptr_t fd_or_handle, Filter* filter, Arguments* arguments, Target* working_directory );
    void stop();
    size_t read( intptr_t fd_or_handle, void* buffer, size_t length ) const;
    void close( intptr_t fd_or_handle ) const;
};

}

}

#endif

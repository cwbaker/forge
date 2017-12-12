#ifndef SWEET_THREAD_JOBQUEUE_HPP_INCLUDED
#define SWEET_THREAD_JOBQUEUE_HPP_INCLUDED

#include "declspec.hpp"
#include "Mutex.hpp"
#include <functional>
#include <vector>

namespace sweet
{
    
namespace thread
{

class SWEET_THREAD_DECLSPEC JobQueue
{
    thread::Mutex jobs_mutex_;
    std::vector<std::function<void ()> > jobs_;
    std::vector<std::function<void ()> > other_jobs_;

    public:
        JobQueue( unsigned int reserve = 0 );
        void add_job( const std::function<void()>& job );
        void dispatch_jobs();
};
    
}

}

#endif

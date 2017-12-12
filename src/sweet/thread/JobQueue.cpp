//
// JobQueue.cpp
// Copyright (c) 2014 Charles Baker.  All rights reserved.
//

#include "JobQueue.hpp"
#include "ScopedLock.hpp"
#include <functional>

using std::vector;
using std::function;
using namespace sweet::thread;

JobQueue::JobQueue( unsigned int reserve )
: jobs_mutex_(),
  jobs_(),
  other_jobs_()
{
    jobs_.reserve( reserve );
    other_jobs_.reserve( reserve );
}

void JobQueue::add_job( const std::function<void()>& job )
{
    ScopedLock lock( jobs_mutex_ );
    jobs_.push_back( job );
}

void JobQueue::dispatch_jobs()
{
    thread::ScopedLock lock( jobs_mutex_ );
    jobs_.swap( other_jobs_ );
    jobs_.clear();
    lock.unlock();

    for ( vector<function<void()> >::const_iterator job = other_jobs_.begin(); job != other_jobs_.end(); ++job )
    {
        (*job)();        
    }
}

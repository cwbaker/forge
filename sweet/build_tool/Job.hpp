//
// Job.hpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_JOB_HPP_INCLUDED
#define SWEET_BUILD_TOOL_JOB_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/pointer/ptr.hpp>
#include <string>

namespace sweet
{

namespace build_tool
{

/**
// The state of a Job.
*/
enum JobState
{
    JOB_WAITING, ///< The Job is waiting in the queue.
    JOB_PROCESSING, ///< The Job is being processed.
    JOB_COMPLETE ///< The Job has been processed.
};

class Environment;
class Target;

/**
// A visit that is queued up and carried out during as postorder traversal
// of a Graph.
*/
class Job
{
    Target* target_; ///< The Target that this Job is for.
    int height_; ///< The height of this Job in its Graph.
    JobState state_; ///< The JobState of this Job.

    public:
        Job( Target* target, int height );

        ptr<Target> get_target() const;
        ptr<Target> get_working_directory() const;
        int get_height() const;

        void set_state( JobState state );
        JobState get_state() const;
        
        bool operator<( const Job& job ) const;

    private:
        void print_failed_dependencies( ptr<Target> target ) const;
};

}

}

#endif

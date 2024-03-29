#ifndef FORGE_JOB_HPP_INCLUDED
#define FORGE_JOB_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace forge
{

/**
// The state of a Job.
*/
enum JobState
{
    JOB_WAITING, ///< The Job is waiting in the queue.
    JOB_PROCESSING, ///< The Job is being processed.
    JOB_COMPLETE ///< The Job is processed.
};

class Target;

/**
// A visit that is queued up and carried out during a postorder traversal
// of a Graph.
*/
class Job
{
    Target* target_; ///< The Target that this Job is for.
    int height_; ///< The height of this Job in its Graph.
    JobState state_; ///< The JobState of this Job.
    bool prune_; ///< Was prune() called during this Job?

    public:
        Job( Target* target, int height = 0 );

        Target* target() const;
        Target* working_directory() const;
        int height() const;
        JobState state() const;
        bool prune() const;
        bool operator<( const Job& job ) const;

        void set_state( JobState state );
        void set_prune( bool prune );
};

}

}

#endif

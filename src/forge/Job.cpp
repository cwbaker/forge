//
// Job.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Job.hpp"
#include "Target.hpp"
#include <assert/assert.hpp>

using namespace sweet;
using namespace sweet::forge;

Job::Job( Target* target, int height )
: target_( target )
, height_( height )
, state_( JOB_WAITING )
, prune_( false )
{
    SWEET_ASSERT( target_ );
    SWEET_ASSERT( height_ >= 0 );
}

Target* Job::target() const
{
    SWEET_ASSERT( target_ );
    return target_;
}

Target* Job::working_directory() const
{
    SWEET_ASSERT( target_ );
    return target_->working_directory();
}

int Job::height() const
{
    SWEET_ASSERT( height_ >= 0 );
    return height_;
}

JobState Job::state() const
{
    SWEET_ASSERT( state_ >= JOB_WAITING && state_ <= JOB_COMPLETE );
    return state_;
}

bool Job::prune() const
{
    return prune_;
}

bool Job::operator<( const Job& job ) const
{
    return height_ < job.height_;
}

void Job::set_state( JobState state )
{
    SWEET_ASSERT( state >= JOB_WAITING && state <= JOB_COMPLETE );
    state_ = state;
}

void Job::set_prune( bool prune )
{
    prune_ = prune;
}

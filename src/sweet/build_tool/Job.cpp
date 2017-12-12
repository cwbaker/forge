//
// Job.cpp
// Copyright (c) 2008 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Job.hpp"
#include "Target.hpp"
#include "Environment.hpp"

using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

Job::Job( Target* target, int height )
: target_( target ),
  height_( height ),
  state_( JOB_WAITING )
{
    SWEET_ASSERT( target_ );
    SWEET_ASSERT( height_ >= 0 );
}

Target* Job::get_target() const
{
    SWEET_ASSERT( target_ );
    return target_;
}

Target* Job::get_working_directory() const
{
    SWEET_ASSERT( target_ );
    return target_->get_working_directory();
}

int Job::get_height() const
{
    SWEET_ASSERT( height_ >= 0 );
    return height_;
}

void Job::set_state( JobState state )
{
    SWEET_ASSERT( state >= JOB_WAITING && state <= JOB_COMPLETE );
    state_ = state;
}

JobState Job::get_state() const
{
    SWEET_ASSERT( state_ >= JOB_WAITING && state_ <= JOB_COMPLETE );
    return state_;
}

bool Job::operator<( const Job& job ) const
{
    return height_ < job.height_;
}

//
// Context.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Context.hpp"
#include "Target.hpp"
#include "BuildTool.hpp"
#include "Graph.hpp"
#include "ScriptInterface.hpp"

using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

/**
// Constructor.
//
// @param index
//  The index of this Context.
//
// @param directory
//  The inital working directory to set for this Context (assumed to be
//  an absolute path).
//
// @param build_tool
//  The BuildTool that this Context is part of.
*/
Context::Context( int index, const fsys::Path& directory, BuildTool* build_tool )
: index_( index ),
  build_tool_( build_tool ),
  context_thread_( build_tool->script_interface()->lua() ),
  working_directory_( NULL ), 
  directories_(), 
  job_( NULL ),
  exit_code_( 0 )
{
    reset_directory( directory );
}

/**
// Get the LuaThread that this Context uses to make script calls.
//
// @return
//  The LuaThread that this Context uses to make script calls.
*/
lua::LuaThread& Context::context_thread()
{
    return context_thread_;
}

/**
// Reset the working directory stack to contain only the directory represented
// by the Target \e directory.
//
// @param directory
//  The Target that represents the directory to set the working directory 
//  stack to contain.
*/
void Context::reset_directory_to_target( Target* directory )
{
    SWEET_ASSERT( directory );
    directories_.clear();
    directories_.push_back( directory->path() );
    working_directory_ = directory;
}

/**
// Reset the working directory stack to contain only \e directory.
//
// @param directory
//  The directory to set the working directory stack to contain (assumed
//  to be absolute or empty).
*/
void Context::reset_directory( const fsys::Path& directory )
{
    SWEET_ASSERT( directory.empty() || directory.is_absolute() );
    directories_.clear();
    directories_.push_back( directory );
    working_directory_ = build_tool_->graph()->target( directory.string() );
}

/**
// Change the current working directory to \e directory.
//
// If \e directory is a relative path then it is considered to be relative
// to the current working directory and an absolute path is constructed by
// combining the current working directory and \e directory.
//
// @param directory
//  The directory to change the current working directory to.
*/
void Context::change_directory( const fsys::Path& directory )
{
    SWEET_ASSERT( !directories_.empty() );

    if ( directory.is_absolute() )
    {
        directories_.back() = directory;
    }
    else
    {
        directories_.back() /= directory;
        directories_.back().normalize();
    }

    working_directory_ = build_tool_->graph()->target( directories_.back().string() );
}

/**
// Push \e directory as the current working directory.
//
// The previous current working directory can be returned to by calling
// Context::pop_directory().
//
// If \e directory is a relative path then it is considered to be relative
// to the current working directory and an absolute path is constructed by
// combining the current working directory and \e directory.
//
// @param directory
//  The directory to change the current working directory to.
*/
void Context::push_directory( const fsys::Path& directory )
{
    SWEET_ASSERT( !directories_.empty() );

    if ( directory.is_absolute() )
    {
        directories_.push_back( directory );
    }
    else
    {
        directories_.push_back( directories_.back() / directory );
        directories_.back().normalize();
    }

    working_directory_ = build_tool_->graph()->target( directories_.back().string() );
}

/**
// Pop the current working directory to return to the previous working 
// directory.
//
// If there is only the current working directory in the directory stack then
// this function silently does nothing.
*/
void Context::pop_directory()
{
    if ( directories_.size() > 1 )
    {
        directories_.pop_back();
        working_directory_ = build_tool_->graph()->target( directories_.back().string() );
    }
}

/**
// Get the current working directory.
//
// @return
//  The current working directory.
*/
const fsys::Path& Context::directory() const
{
    SWEET_ASSERT( !directories_.empty() );
    return directories_.back();
}

/**
// Get the Target that represents the current working directory.
//
// @return
//  The Target that represents the current working directory.
*/
Target* Context::working_directory() const
{
    return working_directory_;
}

/**
// Set the current Job for this Context.
//
// @param job
//  The Job to set as the current Job for this Context or null to set this
//  Context to have no current Job.
*/
void Context::set_job( Job* job )
{
    job_ = job;
}

/**
// Get the current Job for this Context.
//
// @return
//  The current Job or null if this Context doesn't have a current Job.
*/
Job* Context::job() const
{
    return job_;
}

/**
// Set the exit code for this Context.
//
// @param exit_code
//  The value to set the exit code to.
*/
void Context::set_exit_code( int exit_code )
{
    exit_code_ = exit_code;
}

/**
// Get the exit code that is currently set for this Context.
//
// @return
//  The exit code.
*/
int Context::exit_code() const
{
    return exit_code_;
}

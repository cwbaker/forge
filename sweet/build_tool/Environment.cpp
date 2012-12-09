//
// Environment.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Environment.hpp"
#include "Target.hpp"
#include "BuildTool.hpp"
#include "ScriptInterface.hpp"

using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

/**
// Constructor.
//
// @param index
//  The index of this Environment.
//
// @param directory
//  The inital working directory to set for this Environment (assumed to be
//  an absolute path).
//
// @param build_tool
//  The BuildTool that this Environment is part of.
*/
Environment::Environment( int index, const path::Path& directory, BuildTool* build_tool )
: index_( index ),
  build_tool_( build_tool ),
  environment_thread_( build_tool->get_script_interface()->get_lua() ),
  working_directory_(), 
  directories_(), 
  job_( NULL ),
  exit_code_( 0 )
{
//
// Push the working directory at the back of the stack of directories to make
// it the current working directory.
//
    SWEET_ASSERT( directory.empty() || directory.is_absolute() );
    directories_.push_back( directory );
}

/**
// Get the LuaThread that this Environment uses to make script calls.
//
// @return
//  The LuaThread that this Environment uses to make script calls.
*/
lua::LuaThread& Environment::get_environment_thread()
{
    return environment_thread_;
}

/**
// Set the Target to use as the working directory for this Environment.
//
// @param target
//  The Target to set as the relative parent.
*/
void Environment::set_working_directory( ptr<Target> target )
{
    working_directory_ = target;
    if ( target )
    {
        reset_directory( target->get_path() );
    }
}

/**
// Get the Target that is acting as the current relative Target for this 
// Environment.
//
// @return
//  The current relative Target.
*/
ptr<Target> Environment::get_working_directory() const
{
    return working_directory_;
}

/**
// Reset the working directory stack to contain only \e directory.
//
// @param directory
//  The directory to set the working directory stack to contain (assumed
//  to be absolute or empty).
*/
void Environment::reset_directory( const path::Path& directory )
{
    SWEET_ASSERT( directory.empty() || directory.is_absolute() );
    directories_.clear();
    directories_.push_back( directory );
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
void Environment::change_directory( const path::Path& directory )
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
}

/**
// Push \e directory as the current working directory.
//
// The previous current working directory can be returned to by calling
// Environment::pop_directory().
//
// If \e directory is a relative path then it is considered to be relative
// to the current working directory and an absolute path is constructed by
// combining the current working directory and \e directory.
//
// @param directory
//  The directory to change the current working directory to.
*/
void Environment::push_directory( const path::Path& directory )
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
}

/**
// Pop the current working directory to return to the previous working 
// directory.
//
// If there is only the current working directory in the directory stack then
// this function silently does nothing.
*/
void Environment::pop_directory()
{
    if ( directories_.size() > 1 )
    {
        directories_.pop_back();
    }
}

/**
// Get the current working directory.
//
// @return
//  The current working directory.
*/
const path::Path& Environment::get_directory() const
{
    SWEET_ASSERT( !directories_.empty() );
    return directories_.back();
}

/**
// Set the current Job for this Environment.
//
// @param job
//  The Job to set as the current Job for this Environment or null to set this
//  Environment to have no current Job.
*/
void Environment::set_job( Job* job )
{
    job_ = job;
}

/**
// Get the current Job for this Environment.
//
// @return
//  The current Job or null if this Environment doesn't have a current Job.
*/
Job* Environment::get_job() const
{
    return job_;
}

/**
// Set the exit code for this Environment.
//
// @param exit_code
//  The value to set the exit code to.
*/
void Environment::set_exit_code( int exit_code )
{
    exit_code_ = exit_code;
}

/**
// Get the exit code that is currently set for this Environment.
//
// @return
//  The exit code.
*/
int Environment::get_exit_code() const
{
    return exit_code_;
}

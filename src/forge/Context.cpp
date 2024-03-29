//
// Context.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Context.hpp"
#include "Target.hpp"
#include "Forge.hpp"
#include "Graph.hpp"
#include "path_functions.hpp"
#include <luaxx/luaxx.hpp>
#include <lua.hpp>

using namespace sweet;
using namespace sweet::forge;

/**
// Constructor.
//
// @param forge
//  The Forge that this Context is part of.
*/
Context::Context( Forge* forge )
: forge_( forge ),
  lua_state_( nullptr ),
  lua_state_reference_( LUA_NOREF ),
  current_buildfile_( nullptr) ,
  working_directory_( NULL ), 
  directories_(), 
  job_( NULL ),
  exit_code_( 0 ),
  buildfile_calling_context_( nullptr ),
  prune_( false )
{
    lua_State* lua_state = forge->lua_state();
    lua_state_ = lua_newthread( lua_state );
    lua_state_reference_ = luaL_ref( lua_state, LUA_REGISTRYINDEX );
}

Context::~Context()
{
    lua_State* lua_state = forge_->lua_state();
    if ( lua_state )
    {
        luaL_unref( lua_state, LUA_REGISTRYINDEX, lua_state_reference_ );
        lua_state_ = nullptr;
        lua_state_reference_ = LUA_NOREF;
    }
}

/**
// Get the lua_State that this Context uses to make calls.
//
// @return
//  The lua_State
*/
lua_State* Context::lua_state() const
{
    return lua_state_;
}

/**
// Get the current working directory.
//
// @return
//  The current working directory.
*/
const boost::filesystem::path& Context::directory() const
{
    SWEET_ASSERT( !directories_.empty() );
    return directories_.back();
}

/**
// Get the target that represents the currently loading buildfile.
//
// @return
//  The target for the currently loading buildfile or null if there is no 
//  buildfile currently loading (e.g. scripts are executing in the context
//  of a postorder traversal). 
*/
Target* Context::current_buildfile() const
{
    return current_buildfile_;
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
// Get the exit code that is currently set for this Context.
//
// @return
//  The exit code.
*/
int Context::exit_code() const
{
    return exit_code_;
}

Context* Context::buildfile_calling_context() const
{
    return buildfile_calling_context_;
}

bool Context::prune() const
{
    return prune_;
}

/**
// Prepend the working directory to \e path to create an absolute path.
//
// @return
//  The absolute path created by prepending the working directory to 
//  \e path.
*/
boost::filesystem::path Context::absolute( const boost::filesystem::path& path ) const
{
    return sweet::forge::absolute( path, directory() );
}

/**
// Express \e path as a path relative to the working directory.
//
// @return
//  The path to \e path expressed relative to the working directory.
*/
boost::filesystem::path Context::relative( const boost::filesystem::path& path ) const
{
    if ( path.is_relative() )
    {
        return path;
    }
    return sweet::forge::relative( path, directory() );        
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
void Context::reset_directory( const boost::filesystem::path& directory )
{
    SWEET_ASSERT( directory.empty() || directory.is_absolute() );
    directories_.clear();
    directories_.push_back( directory );
    working_directory_ = forge_->graph()->target( directory.string() );
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
void Context::change_directory( const boost::filesystem::path& directory )
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

    working_directory_ = forge_->graph()->target( directories_.back().string() );
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
void Context::push_directory( const boost::filesystem::path& directory )
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

    working_directory_ = forge_->graph()->target( directories_.back().string() );
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
        working_directory_ = forge_->graph()->target( directories_.back().string() );
    }
}

/**
// Set the target that represents the currently loading buildfile.
//
// @param current_buildfile
//  The target to set as the currently loading buildfile or null to reset
//  this context to not having a buildfile loading.
*/
void Context::set_current_buildfile( Target* current_buildfile )
{
    current_buildfile_ = current_buildfile;
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
// Set the exit code for this Context.
//
// @param exit_code
//  The value to set the exit code to.
*/
void Context::set_exit_code( int exit_code )
{
    exit_code_ = exit_code;
}

void Context::set_buildfile_calling_context( Context* context )
{
    buildfile_calling_context_ = context;
}

void Context::set_prune( bool prune )
{
    prune_ = prune;
}

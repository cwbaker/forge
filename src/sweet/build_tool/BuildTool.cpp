//
// BuildTool.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "BuildTool.hpp"
#include "BuildToolEventSink.hpp"
#include "System.hpp"
#include "Scheduler.hpp"
#include "Executor.hpp"
#include "Reader.hpp"
#include "Graph.hpp"
#include "Target.hpp"
#include "Context.hpp"
#include "path_functions.hpp"
#include <sweet/build_tool/build_tool_lua/LuaBuildTool.hpp>
#include <sweet/build_tool/build_tool_lua/LuaTarget.hpp>
#include <sweet/build_tool/build_tool_lua/LuaTargetPrototype.hpp>
#include <sweet/error/ErrorPolicy.hpp>
#include <sweet/assert/assert.hpp>

using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::build_tool;

/**
// Constructor.
//
// @param initial_directory
//  The directory to search up from to find the root directory (assumed to
//  be an absolute path).
//
// @param event_sink
//  The EventSink to fire events from this BuildTool at or null if events 
//  from this BuildTool are to be ignored.
*/
BuildTool::BuildTool( const std::string& initial_directory, error::ErrorPolicy& error_policy, BuildToolEventSink* event_sink )
: error_policy_( error_policy ),
  event_sink_( event_sink ),
  lua_build_tool_( NULL ),
  system_( NULL ),
  reader_( NULL ),
  graph_( NULL ),
  scheduler_( NULL ),
  executor_( NULL ),
  root_directory_(),
  initial_directory_(),
  home_directory_(),
  executable_directory_(),
  stack_trace_enabled_( false )
{
    SWEET_ASSERT( boost::filesystem::path(initial_directory).is_absolute() );

    root_directory_ = make_drive_uppercase( initial_directory );
    initial_directory_ = make_drive_uppercase( initial_directory );
    home_directory_ = make_drive_uppercase( system_->home() );
    executable_directory_ = make_drive_uppercase( system_->executable() ).parent_path();

    lua_build_tool_ = new LuaBuildTool( this );
    system_ = new System;
    reader_ = new Reader( this );
    graph_ = new Graph( this );
    scheduler_ = new Scheduler( this );
    executor_ = new Executor( this );
}

/**
// Destructor
//
// This destructor exists even though it is empty so that the generation of
// code to delete the members is generated in a context in which those 
// classes are defined.
*/
BuildTool::~BuildTool()
{
    delete executor_;
    delete scheduler_;
    delete graph_;
    delete reader_;
    delete system_;
    delete lua_build_tool_;
}

/**
// Get the ErrorPolicy for this BuildTool.
//
// @return
//  The ErrorPolicy;
*/
error::ErrorPolicy& BuildTool::error_policy() const
{
    return error_policy_;
}

/**
// Get the System for this BuildTool.
//
// @return
//  The System.
*/
System* BuildTool::system() const
{
    SWEET_ASSERT( system_ );
    return system_;
}

/**
// Get the Reader for this BuildTool.
//
// @return
//  The Reader.
*/
Reader* BuildTool::reader() const
{
    SWEET_ASSERT( reader_ );
    return reader_;
}

/**
// Get the Graph for this BuildTool.
//
// @return
//  The Graph.
*/
Graph* BuildTool::graph() const
{
    SWEET_ASSERT( graph_ );
    return graph_;
}

/**
// Get the Scheduler for this BuildTool.
//
// @return
//  The Scheduler.
*/
Scheduler* BuildTool::scheduler() const
{
    SWEET_ASSERT( scheduler_ );
    return scheduler_;
}

/**
// Get the Executor for this BuildTool.
//
// @return
//  The Executor.
*/
Executor* BuildTool::executor() const
{
    SWEET_ASSERT( executor_ );
    return executor_;
}

/**
// Get the currently active Context for this BuildTool.
//
// @return
//  The currently active Context.
*/
Context* BuildTool::context() const
{
    SWEET_ASSERT( scheduler_ );
    SWEET_ASSERT( scheduler_->context() );
    return scheduler_->context();
}

lua_State* BuildTool::lua_state() const
{
    SWEET_ASSERT( lua_build_tool_ );
    return lua_build_tool_->lua_state();
}

const boost::filesystem::path& BuildTool::root() const
{
    return root_directory_;
}

boost::filesystem::path BuildTool::root( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( root_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

const boost::filesystem::path& BuildTool::initial() const
{
    return initial_directory_;
}

boost::filesystem::path BuildTool::initial( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( initial_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

const boost::filesystem::path& BuildTool::home() const
{
    return home_directory_;
}

boost::filesystem::path BuildTool::home( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( home_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

const boost::filesystem::path& BuildTool::executable() const
{
    return executable_directory_;
}

boost::filesystem::path BuildTool::executable( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }
    
    boost::filesystem::path absolute_path( executable_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path.string();
}

boost::filesystem::path BuildTool::absolute( const boost::filesystem::path& path ) const
{
    return context()->absolute( path );
}

boost::filesystem::path BuildTool::relative( const boost::filesystem::path& path ) const
{
    return context()->relative( path );
}

/**
// Set whether or not stack traces are reported when an error occurs.
//
// @param
//  True to enable stack traces or false to disable them.
*/
void BuildTool::set_stack_trace_enabled( bool stack_trace_enabled )
{
    stack_trace_enabled_ = stack_trace_enabled;
}

/**
// Is a stack trace reported when an error occurs?
//
// @return
//  True if a stack trace is reported when an error occurs otherwise false.
*/
bool BuildTool::stack_trace_enabled() const
{
    return stack_trace_enabled_;
}

/**
// Set the maximum number of parallel jobs.
//
// @param maximum_parallel_jobs
//  The maximum number of parallel jobs.
*/
void BuildTool::set_maximum_parallel_jobs( int maximum_parallel_jobs )
{
    SWEET_ASSERT( executor_ );
    executor_->set_maximum_parallel_jobs( maximum_parallel_jobs );
}

/**
// Get the maximum number of parallel jobs.
//
// @return
//  The maximum number of parallel jobs.
*/
int BuildTool::maximum_parallel_jobs() const
{
    SWEET_ASSERT( executor_ );
    return executor_->maximum_parallel_jobs();
}

/**
// Set the path to the build hooks library.
//
// @param build_hooks_library
//  The path to the build hooks library or an empty string to disable tracking
//  dependencies via build hooks.
*/
void BuildTool::set_build_hooks_library( const std::string& build_hooks_library )
{
    SWEET_ASSERT( executor_ );
    executor_->set_build_hooks_library( build_hooks_library );
}

/**
// Get the path to the build hooks library.
//
// @return 
//  The path to the build hooks library.
*/
const std::string& BuildTool::build_hooks_library() const
{
    SWEET_ASSERT( executor_ );
    return executor_->build_hooks_library();
}

/**
// Set the root directory to *root_directory*.
//
// @param
//  The absolute path to the root directory.
*/
void BuildTool::set_root_directory( const std::string& root_directory )
{
    root_directory_ = build_tool::absolute( root_directory, initial_directory_ );
}

/**
// Extract assignments from \e assignments_and_commands and use them to 
// assign values to global variables.
//
// This is used to accept variable assignments on the command line and have 
// them available for scripts to use for configuration when commands are
// executed.
//
// @param assignments
//  The assignments specified on the command line used to create global 
//  variables before any scripts are loaded (e.g. 'variant=release' etc).
*/
void BuildTool::assign_global_variables( const std::vector<std::string>& assignments )
{
    SWEET_ASSERT( lua_build_tool_ );
    lua_build_tool_->assign_global_variables( assignments );
}

/**
// Load `build.lua` and execute \e command.
//
// @param filename
//  The name of the file to execute to initialize the build system.
//
// @param command
//  The function to call once the root file has been loaded.
*/
void BuildTool::execute( const std::string& filename, const std::string& command )
{
    error_policy_.push_errors();
    boost::filesystem::path path( root_directory_ / filename );    
    scheduler_->load( path );
    int errors = error_policy_.pop_errors();
    if ( errors == 0 )
    {
        scheduler_->command( path, command );
    }
}

/**
// Compile and run the Lua program contained in *script*.
//
// @param script
//  The Lua program to compile and run.
*/
void BuildTool::script( const std::string& script )
{
    scheduler_->script( root_directory_, script );
}

void BuildTool::create_target_lua_binding( Target* target )
{
    SWEET_ASSERT( lua_build_tool_ );
    lua_build_tool_->lua_target()->create_target( target );
}

void BuildTool::recover_target_lua_binding( Target* target )
{
    SWEET_ASSERT( lua_build_tool_ );
    lua_build_tool_->lua_target()->recover_target( target );
}

void BuildTool::update_target_lua_binding( Target* target )
{
    SWEET_ASSERT( lua_build_tool_ );
    lua_build_tool_->lua_target()->update_target( target );
}

void BuildTool::destroy_target_lua_binding( Target* target )
{
    SWEET_ASSERT( target );
    if ( target && target->referenced_by_script() )
    {
        lua_build_tool_->lua_target()->destroy_target( target );
    }
}

void BuildTool::create_target_prototype_lua_binding( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_build_tool_ );
    lua_build_tool_->lua_target_prototype()->create_target_prototype( target_prototype );
}

void BuildTool::destroy_target_prototype_lua_binding( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( lua_build_tool_ );
    lua_build_tool_->lua_target_prototype()->destroy_target_prototype( target_prototype );
}

/**
// Handle output.
//
// @param text
//  The text to output.
*/
void BuildTool::outputf( const char* format, ... )
{
    SWEET_ASSERT( format );

    if ( event_sink_ && format )
    {
        char message [1024];
        va_list args;
        va_start( args, format );
        vsnprintf( message, sizeof(message), format, args );
        message[sizeof(message) - 1] = 0;
        va_end( args );
        event_sink_->build_tool_output( this, message );
    }
}

/**
// Handle an error message.
//
// @param format
//  A printf style format string that describes the text to output.
//
// @param ...
//  Parameters as specified by \e format.
*/
void BuildTool::errorf( const char* format, ... )
{
    SWEET_ASSERT( format );

    if ( event_sink_ && format )
    {
        char message [1024];
        va_list args;
        va_start( args, format );
        vsnprintf( message, sizeof(message), format, args );
        message[sizeof(message) - 1] = 0;
        va_end( args );
        event_sink_->build_tool_error( this, message );
    }
}

void BuildTool::output( const char* message )
{
    if ( event_sink_ && message )
    {
        event_sink_->build_tool_output( this, message );
    }
}

void BuildTool::error( const char* message )
{
    if ( event_sink_ && message )
    {
        event_sink_->build_tool_error( this, message );
    }
}

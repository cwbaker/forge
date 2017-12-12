//
// BuildTool.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "BuildTool.hpp"
#include "Error.hpp"
#include "BuildToolEventSink.hpp"
#include "OsInterface.hpp"
#include "Scheduler.hpp"
#include "Executor.hpp"
#include "Reader.hpp"
#include "ScriptInterface.hpp"
#include "Graph.hpp"

using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

static const char* ROOT_FILENAME = "build.lua";

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
  warning_level_( 0 ),
  os_interface_( NULL ),
  script_interface_( NULL ),
  reader_( NULL ),
  executor_( NULL ),
  scheduler_( NULL ),
  graph_( NULL )
{
    SWEET_ASSERT( path::Path(initial_directory).is_absolute() );

    os_interface_ = new OsInterface;
    script_interface_ = new ScriptInterface( os_interface_, this );
    reader_ = new Reader( this );
    executor_ = new Executor( this );
    scheduler_ = new Scheduler( this );
    graph_ = new Graph( this );

    script_interface_->set_root_directory( initial_directory );
    script_interface_->set_initial_directory( initial_directory );
    script_interface_->set_executable_directory( path::Path(os_interface_->executable()).branch().string() );
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
    delete graph_;
    delete scheduler_;
    delete executor_;
    delete reader_;
    delete script_interface_;
    delete os_interface_;
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
// Get the OsInterface for this BuildTool.
//
// @return
//  The OsInterface.
*/
OsInterface* BuildTool::os_interface() const
{
    SWEET_ASSERT( os_interface_ );
    return os_interface_;
}

/**
// Get the ScriptInterface for this BuildTool.
//
// @return
//  The ScriptInterface.
*/
ScriptInterface* BuildTool::script_interface() const
{
    SWEET_ASSERT( script_interface_ );
    return script_interface_;
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
// Set the warning level that warnings are reported up to.
//
// Any warnings at a level equal to or less than \e warning_level will be
// reported.  Other warnings will be silently swallowed.
//
// @param warning_level
//  The value to set the warning level to.
*/
void BuildTool::set_warning_level( int warning_level )
{
    warning_level_ = warning_level;
}

/**
// Get the warning level that warnings are reported at.
//
// @return
//  The warning level.
*/
int BuildTool::warning_level() const
{
    return warning_level_;
}

/**
// Set whether or not stack traces are reported when an error occurs.
//
// @param
//  True to enable stack traces or false to disable them.
*/
void BuildTool::set_stack_trace_enabled( bool stack_trace_enabled )
{
    SWEET_ASSERT( script_interface_ );
    script_interface_->lua().set_stack_trace_enabled( stack_trace_enabled );
}

/**
// Is a stack trace reported when an error occurs?
//
// @return
//  True if a stack trace is reported when an error occurs otherwise false.
*/
bool BuildTool::stack_trace_enabled() const
{
    SWEET_ASSERT( script_interface_ );
    return script_interface_->lua().is_stack_trace_enabled();
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
// Find the root directory by searching up the directory hierarchy from the
// initial directory until a directory that contains file 'build.lua' is
// found.
//
// @param directory
//  The directory to start the search from.
*/
void BuildTool::search_up_for_root_directory( const std::string& directory )
{
    boost::filesystem::path root_directory( directory );
    while ( !root_directory.empty() && !os_interface_->exists((root_directory / ROOT_FILENAME).string()) )
    {
        root_directory = root_directory.branch_path();
    }
    if ( !os_interface_->exists((root_directory / ROOT_FILENAME).string()) )
    {
        SWEET_ERROR( RootFileNotFoundError("The file '%s' could not be found to identify the root directory", ROOT_FILENAME) );
    }
    script_interface_->set_root_directory( root_directory.string() );
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
void BuildTool::assign( const std::vector<std::string>& assignments )
{
    for ( std::vector<std::string>::const_iterator i = assignments.begin(); i != assignments.end(); ++i )
    {
        std::string::size_type position = i->find( "=" );
        if ( position != std::string::npos )
        {
            std::string attribute = i->substr( 0, position );
            std::string value = i->substr( position + 1, std::string::npos );
            script_interface_->lua().globals()
                ( attribute.c_str(), value )
            ;
        }
    }
}

/**
// Execute \e filename.
//
// @param filename
//  The path to the script file to execute or an empty string to take the
//  default action of executing the root file 'build.lua'.
//
// @param commands
//  The functions to call once the root file has been loaded.
*/
void BuildTool::execute( const std::string& filename, const std::vector<std::string>& commands )
{
    path::Path path( filename );
    if ( path.empty() )
    {
        path = script_interface_->root_directory() / string( ROOT_FILENAME );
    }
    else if ( path.is_relative() )
    {
        path = script_interface_->initial_directory() / filename;
        path.normalize();
    }
    
    scheduler_->load( path );
    for ( vector<string>::const_iterator command = commands.begin(); command != commands.end(); ++command )
    {
        scheduler_->command( path, *command );
    }
}

/**
// Execute [\e start, \e finish).
//
// @param start
//  The first character of the script to execute.
//
// @param finish
//  One past the last character of the script to execute.
*/
void BuildTool::execute( const char* start, const char* finish )
{
    scheduler_->execute( start, finish );
}

/**
// Handle output.
//
// @param text
//  The text to output.
*/
void BuildTool::output( const char* text )
{
    SWEET_ASSERT( text );

    if ( event_sink_ )
    {
        event_sink_->build_tool_output( this, text );
    }
}

/**
// Handle a warning message.
//
// @param format
//  A printf style format string that describes the text to output.
//
// @param ...
//  Parameters as specified by \e format.
*/
void BuildTool::warning( const char* format, ... )
{
    SWEET_ASSERT( format );

    if ( event_sink_ && warning_level_ > 0 )
    {
        char message [1024];
        va_list args;
        va_start( args, format );
        vsnprintf( message, sizeof(message), format, args );
        message[sizeof(message) - 1] = 0;
        va_end( args );
        event_sink_->build_tool_warning( this, message );
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
void BuildTool::error( const char* format, ... )
{
    SWEET_ASSERT( format );

    if ( event_sink_ )
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

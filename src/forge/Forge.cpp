//
// Forge.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "Forge.hpp"
#include "ForgeEventSink.hpp"
#include "System.hpp"
#include "Scheduler.hpp"
#include "Executor.hpp"
#include "Reader.hpp"
#include "Graph.hpp"
#include "Toolset.hpp"
#include "Target.hpp"
#include "Context.hpp"
#include "path_functions.hpp"
#include <forge/forge_lua/Lua.hpp>
#include <forge/forge_lua/LuaTarget.hpp>
#include <forge/forge_lua/LuaRule.hpp>
#include <forge/forge_lua/LuaToolset.hpp>
#include <error/ErrorPolicy.hpp>
#include <assert/assert.hpp>

using std::string;
using std::vector;
using boost::filesystem::path;
using namespace sweet;
using namespace sweet::forge;

/**
// Constructor.
//
// @param initial_directory
//  The directory to search up from to find the root directory (assumed to
//  be an absolute path).
//
// @param event_sink
//  The EventSink to fire events from this Forge at or null if events 
//  from this Forge are to be ignored.
*/
Forge::Forge( const std::string& initial_directory, error::ErrorPolicy& error_policy, ForgeEventSink* event_sink )
: error_policy_( error_policy )
, event_sink_( event_sink )
, lua_( nullptr )
, system_( nullptr )
, reader_( nullptr )
, graph_( nullptr )
, scheduler_( nullptr )
, executor_( nullptr )
, root_directory_()
, initial_directory_()
, home_directory_()
, executable_directory_()
, stack_trace_enabled_( false )
{
    SWEET_ASSERT( boost::filesystem::path(initial_directory).is_absolute() );

    root_directory_ = make_drive_uppercase( initial_directory );
    initial_directory_ = make_drive_uppercase( initial_directory );
    home_directory_ = make_drive_uppercase( system_->home() );
    executable_directory_ = make_drive_uppercase( system_->executable() ).parent_path();
}

/**
// Destructor
//
// This destructor exists even though it is empty so that the generation of
// code to delete the members is generated in a context in which those 
// classes are defined.
*/
Forge::~Forge()
{
    destroy();
}

/**
// Get the ErrorPolicy for this Forge.
//
// @return
//  The ErrorPolicy;
*/
error::ErrorPolicy& Forge::error_policy() const
{
    return error_policy_;
}

/**
// Get the System for this Forge.
//
// @return
//  The System.
*/
System* Forge::system() const
{
    SWEET_ASSERT( system_ );
    return system_;
}

/**
// Get the Reader for this Forge.
//
// @return
//  The Reader.
*/
Reader* Forge::reader() const
{
    SWEET_ASSERT( reader_ );
    return reader_;
}

/**
// Get the Graph for this Forge.
//
// @return
//  The Graph.
*/
Graph* Forge::graph() const
{
    SWEET_ASSERT( graph_ );
    return graph_;
}

/**
// Get the Scheduler for this Forge.
//
// @return
//  The Scheduler.
*/
Scheduler* Forge::scheduler() const
{
    SWEET_ASSERT( scheduler_ );
    return scheduler_;
}

/**
// Get the Executor for this Forge.
//
// @return
//  The Executor.
*/
Executor* Forge::executor() const
{
    SWEET_ASSERT( executor_ );
    return executor_;
}

/**
// Get the currently active Context for this Forge.
//
// @return
//  The currently active Context.
*/
Context* Forge::context() const
{
    SWEET_ASSERT( scheduler_ );
    SWEET_ASSERT( scheduler_->context() );
    return scheduler_->context();
}

lua_State* Forge::lua_state() const
{
    SWEET_ASSERT( lua_ );
    return lua_->lua_state();
}

const boost::filesystem::path& Forge::root() const
{
    return root_directory_;
}

boost::filesystem::path Forge::root( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( root_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

const boost::filesystem::path& Forge::initial() const
{
    return initial_directory_;
}

boost::filesystem::path Forge::initial( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( initial_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

const boost::filesystem::path& Forge::home() const
{
    return home_directory_;
}

boost::filesystem::path Forge::home( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }

    boost::filesystem::path absolute_path( home_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

const boost::filesystem::path& Forge::executable() const
{
    return executable_directory_;
}

boost::filesystem::path Forge::executable( const boost::filesystem::path& path ) const
{
    if ( boost::filesystem::path(path).is_absolute() )
    {
        return path;
    }
    
    boost::filesystem::path absolute_path( executable_directory_ );
    absolute_path /= path;
    absolute_path.normalize();
    return absolute_path;
}

boost::filesystem::path Forge::absolute( const boost::filesystem::path& path ) const
{
    return context()->absolute( path );
}

boost::filesystem::path Forge::relative( const boost::filesystem::path& path ) const
{
    return context()->relative( path );
}

/**
// Set whether or not stack traces are reported when an error occurs.
//
// @param
//  True to enable stack traces or false to disable them.
*/
void Forge::set_stack_trace_enabled( bool stack_trace_enabled )
{
    stack_trace_enabled_ = stack_trace_enabled;
}

/**
// Is a stack trace reported when an error occurs?
//
// @return
//  True if a stack trace is reported when an error occurs otherwise false.
*/
bool Forge::stack_trace_enabled() const
{
    return stack_trace_enabled_;
}

/**
// Set the maximum number of parallel jobs.
//
// @param maximum_parallel_jobs
//  The maximum number of parallel jobs.
*/
void Forge::set_maximum_parallel_jobs( int maximum_parallel_jobs )
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
int Forge::maximum_parallel_jobs() const
{
    SWEET_ASSERT( executor_ );
    return executor_->maximum_parallel_jobs();
}

/**
// Set the path to the build hooks library.
//
// @param forge_hooks_library
//  The path to the build hooks library or an empty string to disable tracking
//  dependencies via build hooks.
*/
void Forge::set_forge_hooks_library( const std::string& forge_hooks_library )
{
    SWEET_ASSERT( executor_ );
    executor_->set_forge_hooks_library( forge_hooks_library );
}

/**
// Get the path to the build hooks library.
//
// @return 
//  The path to the build hooks library.
*/
const std::string& Forge::forge_hooks_library() const
{
    SWEET_ASSERT( executor_ );
    return executor_->forge_hooks_library();
}

/**
// Set the root directory to *root_directory*.
//
// @param
//  The absolute path to the root directory.
*/
void Forge::set_root_directory( const std::string& root_directory )
{
    root_directory_ = forge::absolute( root_directory, initial_directory_ );
}

/**
// Set the Lua module search path in `package.path`.
//
// @param path
//  The value to set `package.path` to.
*/
void Forge::set_package_path( const std::string& path )
{
    SWEET_ASSERT( lua_ );
    lua_->set_package_path( path );
}

void Forge::reset()
{
    destroy();

    lua_ = new Lua( this );
    system_ = new System;
    reader_ = new Reader( this );
    graph_ = new Graph( this );
    scheduler_ = new Scheduler( this );
    executor_ = new Executor( this );

#if defined BUILD_OS_WINDOWS
    set_forge_hooks_library( executable("forge_hooks.dll").generic_string() );
#elif defined BUILD_OS_MACOS
    set_forge_hooks_library( executable("forge_hooks.dylib").generic_string() );
#else
    set_forge_hooks_library( executable("libforge_hooks.so").generic_string() );
#endif

    set_maximum_parallel_jobs( 2 * system_->number_of_logical_processors() );
}

void Forge::destroy()
{
    delete executor_;
    delete scheduler_;
    delete graph_;
    delete reader_;
    delete system_;
    delete lua_;
}

/**
// Load and execute *filename*.
//
// @param filename
//  The name of the file to load and execute.
//
// @return
//  The number of errors that occurred loading and executing the file.
*/
int Forge::file( const std::string& filename )
{
    error_policy_.push_errors();
    boost::filesystem::path path( root_directory_ / filename );    
    scheduler_->load( path );
    return error_policy_.pop_errors();
}

/**
// Execute the global Lua function *command*.
//
// @param command
//  The global Lua function to call to execute the command.
//
// @return
//  The number of errors that occurred executing the command.
*/
int Forge::command( const std::vector<std::string>& assignments, const std::string& build_script, const std::string& command )
{
    reset();
    error_policy_.push_errors();
    lua_->assign_variables( assignments );
    scheduler_->load( path(root_directory_ / build_script) );
    if ( error_policy_.errors() == 0 )
    {
        scheduler_->command( root_directory_, command );
    }
    return error_policy_.pop_errors();
}

/**
// Compile and run the Lua program contained in *script*.
//
// @param script
//  The Lua program to compile and run.
*/
int Forge::script( const std::string& script )
{
    reset();
    error_policy_.push_errors();
    scheduler_->script( root_directory_, script );
    return error_policy_.pop_errors();
}

void Forge::create_target_lua_binding( Target* target )
{
    SWEET_ASSERT( lua_ );
    lua_->lua_target()->create_target( target );
}

void Forge::update_target_lua_binding( Target* target )
{
    SWEET_ASSERT( lua_ );
    lua_->lua_target()->update_target( target );
}

void Forge::destroy_target_lua_binding( Target* target )
{
    SWEET_ASSERT( target );
    if ( target && target->referenced_by_script() )
    {
        lua_->lua_target()->destroy_target( target );
    }
}

void Forge::create_toolset_lua_binding( Toolset* toolset )
{
    SWEET_ASSERT( toolset );
    if ( toolset )
    {
        lua_->lua_toolset()->create_toolset( toolset );
    }
}

void Forge::destroy_toolset_lua_binding( Toolset* toolset )
{
    SWEET_ASSERT( toolset );
    if ( toolset )
    {
        lua_->lua_toolset()->destroy_toolset( toolset );
    }
}

void Forge::create_rule_lua_binding( Rule* rule )
{
    SWEET_ASSERT( lua_ );
    lua_->lua_rule()->create_rule( rule );
}

void Forge::destroy_rule_lua_binding( Rule* rule )
{
    SWEET_ASSERT( lua_ );
    lua_->lua_rule()->destroy_rule( rule );
}

/**
// Handle output.
//
// @param text
//  The text to output.
*/
void Forge::outputf( const char* format, ... )
{
    SWEET_ASSERT( format );

    if ( event_sink_ && format )
    {
        char message [8192];
        va_list args;
        va_start( args, format );
        vsnprintf( message, sizeof(message), format, args );
        message[sizeof(message) - 1] = 0;
        va_end( args );
        event_sink_->forge_output( this, message );
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
void Forge::errorf( const char* format, ... )
{
    SWEET_ASSERT( format );

    if ( event_sink_ && format )
    {
        char message [8192];
        va_list args;
        va_start( args, format );
        vsnprintf( message, sizeof(message), format, args );
        message[sizeof(message) - 1] = 0;
        va_end( args );
        event_sink_->forge_error( this, message );
    }
}

void Forge::output( const char* message )
{
    if ( event_sink_ && message )
    {
        event_sink_->forge_output( this, message );
    }
}

void Forge::error( const char* message )
{
    if ( event_sink_ && message )
    {
        event_sink_->forge_error( this, message );
    }
}

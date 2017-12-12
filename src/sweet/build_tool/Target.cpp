//
// Target.cpp
// Copyright (c) 2007 - 2013 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Target.hpp"
#include "Error.hpp"
#include "TargetPrototype.hpp"
#include "Graph.hpp"
#include "BuildTool.hpp"
#include "ScriptInterface.hpp"
#include "OsInterface.hpp"

using std::swap;
using std::vector;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

/**
// Constructor.
*/
Target::Target()
: pointer::enable_ptr_from_this<Target>(),
  id_(),
  path_(),
  branch_(),
  graph_( NULL ),
  prototype_(),
  timestamp_( 0 ),
  last_write_time_( 0 ),
  last_scan_time_( 0 ),
  outdated_( false ),
  changed_( false ),
  bound_to_file_( false ),
  bound_to_dependencies_( false ),
  referenced_by_script_( false ),
  required_to_exist_( false ),
  filename_(),
  working_directory_(),
  parent_(),
  targets_(),
  dependencies_(),
  explicit_dependencies_( 0 ),
  visiting_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 ),
  postorder_height_( -1 ),
  anonymous_( 0 )
{
}

/**
// Constructor.
//
// @param id
//  The id for this Target.
//
// @param graph
//  The Graph that this Target is part of.
*/
Target::Target( const std::string& id, Graph* graph )
: pointer::enable_ptr_from_this<Target>(),
  id_( id ),
  path_(),
  branch_(),
  graph_( graph ),
  prototype_(),
  timestamp_( 0 ),
  last_write_time_( 0 ),
  last_scan_time_( 0 ),
  outdated_( false ),
  changed_( false ),
  bound_to_file_( false ),
  bound_to_dependencies_( false ),
  referenced_by_script_( false ),
  required_to_exist_( false ),
  filename_(),
  working_directory_(),
  parent_(),
  targets_(),
  dependencies_(),
  explicit_dependencies_( 0 ),
  visiting_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 ),
  postorder_height_( -1 ),
  anonymous_( 0 )
{
    SWEET_ASSERT( graph_ );
}

/**
// Recover this Target after it has been loaded from an Archive.
//
// @param graph
//  The Graph that this Target is part of.
*/
void Target::recover( Graph* graph )
{
    SWEET_ASSERT( graph );    
    SWEET_ASSERT( graph_ == NULL || graph_ == graph );

    graph_ = graph;

    if ( referenced_by_script_ )
    {
        graph->get_build_tool()->get_script_interface()->recover_target( ptr_from_this() );
        graph->get_build_tool()->get_script_interface()->update_target( ptr_from_this(), get_prototype() );
    }

    for ( vector<ptr<Target> >::const_iterator i = targets_.begin(); i != targets_.end(); ++i )
    {
        Target* target = i->get();
        SWEET_ASSERT( target );
        target->parent_ = ptr_from_this();
        target->recover( graph );
    }
}

/**
// Get the id of this Target.
//
// @return
//  The id or the empty string if this Target doesn't have an 
//  id.
*/
const std::string& Target::get_id() const
{
    return id_;
}

/**
// Get the full path to this Target.
//
// @return
//  The full path to this Target.
*/
const std::string& Target::get_path() const
{
    if ( path_.empty() )
    {
        path_ = get_branch() + get_id();
    }

    return path_;
}

/**
// Get the branch path to this Target is in.
//
// @return
//  The branch path that this target is in.
*/
const std::string& Target::get_branch() const
{
    if ( branch_.empty() )
    {
        vector<ptr<Target> > targets_to_root;

        ptr<Target> parent = get_parent();
        while ( parent )
        {
            targets_to_root.push_back( parent );
            parent = parent->get_parent();
        }

        if ( !targets_to_root.empty() )
        {
            vector<ptr<Target> >::const_reverse_iterator i = targets_to_root.rbegin();
            ++i;

            if ( i == targets_to_root.rend() || (*i)->get_id().find(path::BasicPathTraits<char>::DRIVE) == std::string::npos )
            {
                branch_ += "/";
            }

            while ( i != targets_to_root.rend() )
            {
                Target* target = i->get();
                SWEET_ASSERT( target != 0 );
                branch_ += target->get_id() + "/";
                ++i;
            }
        }
    }

    return branch_;
}

/**
// Get the Graph that this Target is part of.
//
// @return
//  The Graph.
*/
Graph* Target::get_graph() const
{
    SWEET_ASSERT( graph_ );
    return graph_;
}

/**
// Set the TargetPrototype for this Target.
//
// @param target_prototype
//  The TargetPrototype to set this Target to have or null to set this Target to have
//  no TargetPrototype.
*/
void Target::set_prototype( ptr<TargetPrototype> target_prototype )
{
    SWEET_ASSERT( !prototype_ || prototype_ == target_prototype );    
    if ( !prototype_ || prototype_ != target_prototype )
    {
        prototype_ = target_prototype;
        if ( is_referenced_by_script() )
        {
            graph_->get_build_tool()->get_script_interface()->update_target( ptr_from_this(), target_prototype );
        }
    }
}

/**
// Get the TargetPrototype for this Target.
//
// @return
//  The TargetPrototype or null if this Target has no TargetPrototype.
*/
ptr<TargetPrototype> Target::get_prototype() const
{
    return prototype_;
}

/**
// Bind this Target.
*/
void Target::bind()
{    
    bind_to_file();
    bind_to_dependencies();
}

/**
// Bind this Target to a file.
//
// If the file doesn't exist then the timestamp is set to the latest possible 
// time so that this Target will always be newer than any of the Targets that 
// depend on it.
//
// If the file exists and is a directory then the timestamp is set to the 
// earliest possible time so that it won't cause any of the Targets that 
// depend on this Target to be outdated (as they simply require that the 
// directory exists).
//
// If the file exists and is a file then the timestamp of this Target is 
// set to the last write time of the file so that Targets that depend on this 
// Target will be outdated if they are older than this Target.  Additionally
// if the last write time of the file is different to the last write time 
// already stored in this Target then this Target is marked as having 
// changed so that it can be scanned if necessary.
//
// Otherwise the file doesn't exist or if this Target's filename is an empty 
// string the timestamp of this Target is set to the earliest possible time 
// so that it will always be older than any of its dependencies.
*/
void Target::bind_to_file()
{
    if ( !bound_to_file_ )
    {
        if ( !filename_.empty() )
        {
            OsInterface* os_interface = graph_->get_build_tool()->get_os_interface();
            if ( os_interface->exists(filename_) )
            {
                if ( os_interface->is_regular(filename_) )
                {
                    std::time_t last_write_time = os_interface->last_write_time( filename_ );
                    changed_ = last_write_time_ != last_write_time;
                    timestamp_ = last_write_time;
                    last_write_time_ = last_write_time;
                }
                outdated_ = false;
            }
            else
            {
                changed_ = last_write_time_ != 0;
                timestamp_ = std::numeric_limits<time_t>::max();
                last_write_time_ = 0;
                outdated_ = true;
            }
        }
        else
        {
            changed_ = last_write_time_ != 0;
            timestamp_ = 0;
            last_write_time_ = 0;
            outdated_ = false;
        }
        
        bound_to_file_ = true;
    }
}

/**
// Bind this Target to its dependencies.
//
// Sets the timestamp of this Target to be the latest of the last write
// time of the file that it is bound to or the latest timestamp of any of 
// its dependencies and sets this Target to be outdated if any of its 
// dependencies have a timestamp that is later than its last write time.
*/
void Target::bind_to_dependencies()
{
    if ( !bound_to_dependencies_ )
    {
        time_t timestamp = timestamp_;
        bool outdated = outdated_;

        if ( !filename_.empty() )
        {
            for ( vector<Target*>::const_iterator i = dependencies_.begin(); i != dependencies_.end(); ++i )
            {
                Target* dependency = *i;
                SWEET_ASSERT( dependency );
                outdated = outdated || dependency->get_timestamp() > get_last_write_time();
                timestamp = std::max( timestamp, dependency->get_timestamp() );
            }
        }
        else
        {
            for ( vector<Target*>::const_iterator i = dependencies_.begin(); i != dependencies_.end(); ++i )
            {
                Target* dependency = *i;
                SWEET_ASSERT( dependency );
                outdated = outdated || dependency->is_outdated();
                timestamp = std::max( timestamp, dependency->get_timestamp() );
            }
        }

        set_timestamp( timestamp );
        set_outdated( outdated );
        bound_to_dependencies_ = true;
    }
}

/**
// Set whether or not this Target is referenced by a scripting object.
//
// Targets that are referenced by scripting objects are passed to the 
// scripting interface for additional clean up when they are destroyed.
//
// @param referenced_by_script
//  True to set this Target as being referenced by a scripting object or false
//  otherwise.
*/
void Target::set_referenced_by_script( bool referenced_by_script )
{
    referenced_by_script_ = referenced_by_script;
}

/**
// Is this Target referenced by a scripting object?
//
// @return
//  True if this Target is referenced by a scripting object otherwise false.
*/
bool Target::is_referenced_by_script() const
{
    return referenced_by_script_;
}

/**
// Set whether or not this Target is required to be bound to an existing file.
//
// @param required_to_exist
//  True to require that this Target is bound to an existing file (i.e. it is
//  a source file that must exist for the build to succeed) otherwise 
//  false to allow this Target to be bound to files that don't exist (i.e. it
//  is a file that is possibly generated by the build process).
*/
void Target::set_required_to_exist( bool required_to_exist )
{
    required_to_exist_ = required_to_exist;
}

/**
// Is this Target required to be bound to an existing file?
//
// @return
//  True if this Target is required to be bound to an existing file otherwise
//  false.
*/
bool Target::is_required_to_exist() const
{
    return required_to_exist_;
}

/**
// Set the timestamp for this Target.
//
// The timestamp is the time that is used to determine whether generated
// Targets are outdated with respect to their dependencies.  Generated Targets
// that have any dependencies with a timestamp later than theirs are 
// considered to be outdated and in need of update.
//
// @param timestamp
//  The value to set the timestamp of this Target to.
*/
void Target::set_timestamp( std::time_t timestamp )
{
    timestamp_ = timestamp;
}

/**
// Get the timestamp of this Target.
//
// If this Target isn't bound to a file then the last write time is always
// 0 (January 1st, 1970, 00:00 GMT).  Because this should be older than any 
// of the dependencies this will leave unbound targets always needing to be
// updated.
//
// @return
//  The timestamp.
*/
std::time_t Target::get_timestamp() const
{
    return timestamp_;
}

/**
// Get the last write time of the file that this Target is bound to.
//
// If this Target hasn't been bound to a file or it has been bound to a file
// that doesn't exist then the last write time will be 0 (January 1st, 1970, 
// 00:00 GMT).  This can be used to determine whether or not a file exists
// after it has been bound.
//
// @return
//  The last write time of the file that this Target is bound to.
*/
std::time_t Target::get_last_write_time() const
{
    return last_write_time_;
}

/**
// Set the last scan time of the file that this Target is bound to.
//
// @param last_scan_time
//  The value to set the last scan time of this Target to.
*/
void Target::set_last_scan_time( std::time_t last_scan_time )
{
    last_scan_time_ = last_scan_time;
}

/**
// Get the last scan time of this Target.
//
// @return
//  The last scan time of this Target.
*/
std::time_t Target::get_last_scan_time() const
{
    return last_scan_time_;
}

/**
// Set whether or not this Target is out of date.
//
// @param outdated
//  Whether or not to set this Target as being out of date.
*/
void Target::set_outdated( bool outdated )
{
    outdated_ = outdated;
}

/**
// Is this Target out of date?
//
// @return
//  True if this Target is out of date otherwise false.
*/
bool Target::is_outdated() const
{
    return outdated_;
}

/**
// Is this Target's last write time changed since it was last bound?
//
// @return
//  True if this Target's last write time has changed between its previous
//  and most recent bindings.
*/
bool Target::is_changed() const
{
    return changed_;
}

/**
// Is this Target bound?
//
// @return
//  True if this Target is bound otherwise false.
*/
bool Target::is_bound_to_file() const
{
    return bound_to_file_;
}

/**
// Set the filename that this Target is bound to.
//
// @param filename
//  The name of the file to bind this Target to or an empty string to set 
//  this Target not to be bound to any file.
*/
void Target::set_filename( const std::string& filename )
{
    filename_ = filename;
}

/**
// Get the filename that this Target is bound to.
//
// @return
//  The name of the file that this Target is bound to or an empty string if 
//  this Target isn't bound to a file.
*/
const std::string& Target::get_filename() const
{
    return filename_;
}

/**
// Set the working_directory of this Target.
//
// The working directory is the Target that specifies the directory that files
// specified in scripts for the Target are relative to by default and that 
// any commands executed by the Target's script functions are started in.
//
// If a Target has no relative Target then the root Target is used instead.
//
// @param target
//  The Target to set as this Target's working directory or null to set this
//  Target to use the root Target as its working directory.
*/
void Target::set_working_directory( ptr<Target> target )
{
    working_directory_ = target;
}

/**
// Get the relative parent of this Target.
//
// @return
//  The relative parent of this Target or null if this Target doesn't have a
//  relative parent.
*/
ptr<Target> Target::get_working_directory() const
{
    return working_directory_.lock();
}

/**
// Set the Target that is a parent of this Target in the Target namespace.
//
// @param target
//  The Target to make a parent of this Target.
*/
void Target::set_parent( ptr<Target> target )
{
    parent_ = target;
}

/**
// Get the Target that is a parent of this Target.
//
// @return
//  The Target that is a parent of this Target.
*/
ptr<Target> Target::get_parent() const
{
    return parent_.lock();
}

/**
// Add a Target as a child of this Target.
//
// Assumes that \e target isn't null, \e target isn't already a child of this 
// Target and that there isn't already another Target with the same 
// identifier in this Target.
//
// @param target
//  The Target to add as a child of this Target.
//
// @param this_target
//  This Target.
*/
void Target::add_target( ptr<Target> target, ptr<Target> this_target )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( this_target.get() == this );
    SWEET_ASSERT( std::find(targets_.begin(), targets_.end(), target) == targets_.end() );
    SWEET_ASSERT( target->get_id().empty() || !find_target_by_id(target->get_id()) );
    SWEET_ASSERT( !target->get_parent() );

    targets_.push_back( target );
    target->set_parent( this_target );
}

/**
// Find a Target by id.
//
// @param id
//  The identifier of the Target to find.
//
// @return
//  The Target or null if no matching Target could be found.
*/
ptr<Target> Target::find_target_by_id( const std::string& id ) const
{
    vector<ptr<Target> >::const_iterator i = targets_.begin();
    while ( i != targets_.end() && (*i)->get_id() != id )
    {
        ++i;
    }
    return i != targets_.end() ? *i : ptr<Target>();
}

/**
// Get the Targets that are part of this Target.
//
// @return
//  The Targets.
*/
const std::vector<ptr<Target> >& Target::get_targets() const
{
    return targets_;
}

/**
// Add a Target as a dependency of this Target.
//
// If \e target is null or is already a dependency of this Target then this 
// function silently does nothing.  This allows calling code to be simpler
// in that optional dependencies that are not enabled can be implemented by
// setting the optional dependency Target to null adding that as a dependency
// and it also saves calling code from having to make sure that it doesn't
// add dependencies twice.
//
// Dependencies that create cycles in the Graph aren't checked for by this 
// function but are detected and reported during Graph traversal.  The 
// traversal continues as if the cyclic dependency was built successfully.
//
// Adding a dependency clears the bound to dependencies flag for this Target 
// because its outdated flag and timestamp have potentially changed because it
// may now have outdated and/or newer dependencies.
//
// The count of explicit dependencies is incremented if this Target's Graph is
// in explicit dependency mode after being cleared (see Graph::clear() and 
// Graph::mark_future_dependencies_as_implicit()).
//
// @param target
//  The Target to add as a dependency.
*/
void Target::add_dependency( ptr<Target> target )
{
    if ( target && target.get() != this )
    {
        SWEET_ASSERT( target->get_graph() == get_graph() );
        if ( !is_dependency(target) )
        {
            dependencies_.push_back( target.get() );
            bound_to_dependencies_ = false;
            if ( !graph_->implicit_dependencies() )
            {
                ++explicit_dependencies_;
            }
        }
    }
}

/**
// Remove a dependency of this Target.
//
// If \e target is null or is not a dependency of this Target then this 
// function silently does nothing.  This allows calling code to be simpler in
// that it doesn't need to check for null.
//
// Removing a dependency clears the bound to dependencies flag for this Target
// because its outdated flag and/or timestamp have potentially changed because
// the dependency that effectively determined this Target's outdatedness or 
// timestamp is no longer a dependency.
//
// @param target
//  The Target to remove as a dependency.
*/
void Target::remove_dependency( ptr<Target> target )
{
    if ( target && target.get() != this )
    {
        SWEET_ASSERT( target->get_graph() == get_graph() );
        vector<Target*>::iterator i = find( dependencies_.begin(), dependencies_.end(), target.get() );
        if ( i != dependencies_.end() )
        {
            swap( *i, dependencies_.back() );
            dependencies_.pop_back();
        }
    }
}

/**
// Remove all implicit dependencies from this Target.
//
// Removing a dependency clears the bound to dependencies flag for this Target 
// because its outdated flag and timestamp have potentially changed because it
// may now not have outdated and/or newer dependencies.
*/
void Target::clear_implicit_dependencies()
{
    dependencies_.erase( dependencies_.begin() + explicit_dependencies_, dependencies_.end() );
    bound_to_dependencies_ = false;
}

/**
// Is a Target a dependency of this Target?
//
// @param target
//  The Target to check for being a dependency of this Target.
//
// @return
//  True if this Target is a dependency otherwise false.
*/
bool Target::is_dependency( ptr<Target> target ) const
{
    SWEET_ASSERT( target && target->get_graph() == get_graph() );
    return target && std::find( dependencies_.begin(), dependencies_.end(), target.get() ) != dependencies_.end();
}

/**
// Are all of the dependencies of this Target built successfully?
//
// @return
//  True if all of this Target's dependencies were built successfully.
*/
bool Target::is_buildable() const
{
    bool successful = true;
    vector<Target*>::const_iterator i = dependencies_.begin(); 
    while ( i != dependencies_.end() && successful )
    {
        Target* target = *i;
        SWEET_ASSERT( target );
        if ( !target->is_successful() )
        {
            successful = false;
        }        
        ++i;
    }
    return successful;
}

/**
// Generate a string containing the dependencies of this Target that failed 
// in the current traversal.
//
// @return
//  A string containing the failed dependencies.
*/
std::string Target::generate_failed_dependencies_message() const
{
    SWEET_ASSERT( !is_buildable() );

//
// Append the identifier for this Target.
//
    std::string message;
    if ( !get_id().empty() )
    {
        message += "'" + get_id() + "'";
    }
    else if ( get_prototype() != 0 )
    {
        message += get_prototype()->get_id();
    }
    else
    {
        message += "<anonymous>";
    }

//
// Append the body of the message.
//
    message += " failed for lack of ";

//
// Append the first failed dependency.
//
    vector<Target*>::const_iterator i = dependencies_.begin();
    while ( i != dependencies_.end() && (*i)->is_successful() )
    {
        ++i;
    }
    
    SWEET_ASSERT( i != dependencies_.end() );    
    if ( i != dependencies_.end() )
    {
        Target* dependency = *i;
        SWEET_ASSERT( dependency );
        if ( !dependency->get_id().empty() )
        {
            message += "'" + dependency->get_id() + "'";
        }
        else if ( dependency->get_prototype() != 0 )
        {
            message += dependency->get_prototype()->get_id();
        }
        else
        {
            message += "<anonymous>";
        }

        ++i;
    }

//
// Append the remaining failed dependencies.
//
    while ( i != dependencies_.end() )
    {
        Target* dependency = *i;
        SWEET_ASSERT( dependency );
        
        if ( !dependency->is_successful() )
        {
            if ( !dependency->get_id().empty() )
            {
                message += ", '" + dependency->get_id() + "'";
            }
            else if ( dependency->get_prototype() != 0 )
            {
                message += ", " + dependency->get_prototype()->get_id();
            }
            else
            {
                message += ", <anonymous>";
            }
        }

        ++i;
    }

    return message;
}

/**
// Get the dependencies of this Target.
//
// The dependencies of a Target are the other Targets that must be built 
// before this Target can be built.
//
// @return
//  The dependencies.
*/
const std::vector<Target*>& Target::get_dependencies() const
{
    return dependencies_;
}

/**
// Set whether or not this Target is in the process of being visited.
//
// @param visiting
//  The value to set whether or not this Target is being visited to.
*/
void Target::set_visiting( bool visiting )
{
    SWEET_ASSERT( visiting_ != visiting );
    visiting_ = visiting;
}

/**
// Is this Target being visited?
//
// @return
//  True if this Target is currently in the process of being visited (it or
//  any of its dependencies are being visited) otherwise false.
*/
bool Target::is_visiting() const
{
    return visiting_;
}

/**
// Mark this Target as having been visited or not in the current pass.
//
// @param visited
//  Whether to mark this Target as being visited or not.
*/
void Target::set_visited( bool visited )
{
    SWEET_ASSERT( graph_ );
    visited_revision_ = visited ? graph_->get_visited_revision() : graph_->get_visited_revision() - 1;
}

/**
// Is this Target marked as having been visited in the current pass?
//
// @return
//  True if this Target has been visited in the current pass otherwise false.
*/
bool Target::is_visited() const
{
    SWEET_ASSERT( graph_ );
    return visited_revision_ == graph_->get_visited_revision();
}

/**
// Marks this Target as having been visited successfully in the current pass.
//
// @param visited
//  Whether to mark this Target as being visited successfully or not.
*/
void Target::set_successful( bool successful )
{
    SWEET_ASSERT( graph_ );
    successful_revision_ = successful ? graph_->get_successful_revision() : graph_->get_successful_revision() - 1;
}

/**
// Has this Target been visited successfully in the current pass?
//
// @return
//  True if this Target has been visited successfully in the current pass.
*/
bool Target::is_successful() const
{
    SWEET_ASSERT( graph_ );
    return successful_revision_ == graph_->get_successful_revision();
}

/**
// Set the postorder height of this Target.
//
// The postorder height is the height, measured in Targets, from the leaves to
// the root.  That is any target without dependencies has a height of 0; any
// target with dependencies has a height equal to the maximum height of any of
// its dependencies plus 1.  This gives leaves a height of 0 and makes the 
// root of the highest node.
//
// @param height
//  The value to set the postorder height of this Target to.
*/
void Target::set_postorder_height( int height )
{
    postorder_height_ = height;
}

/**
// Get the postorder height of this Target.
//
// @return
//  The postorder height of this Target.
*/
int Target::postorder_height() const
{
    return postorder_height_;
}

/**
// Get the next anonymous index from this Target.
//
// Returns the next index that can be used to generate an anonymous identifier
// for the children of this Target.  Each call increments the anonymous index.
//
// @return
//  The next anonymous index.
*/
int Target::anonymous()
{
    return anonymous_++;
}

/**
// Push a std::time_t onto the Lua stack.
//
// @param lua_state
//  The lua_State to push the std::time_t onto the stack of.
//
// @param timestamp
//  The std::time_t to push.
//
// @relates Target
*/
void sweet::lua::lua_push( lua_State* lua_state, std::time_t timestamp )
{
    SWEET_ASSERT( lua_state );
    lua_pushnumber( lua_state, static_cast<lua_Number>(timestamp) );
}

/**
// Convert a number on the Lua stack into a std::time_t.
//
// @param lua_state
//  The lua_State to get the std::time_t from.
//
// @param position
//  The position of the std::time_t on the stack.
//
// @param null_pointer_for_overloading
//  Ignored.
//
// @return
//  The std::time_t.
//
// @relates Target
*/
std::time_t sweet::lua::lua_to( lua_State* lua_state, int position, const std::time_t* null_pointer_for_overloading )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_isnumber(lua_state, position) );
    return static_cast<std::time_t>( lua_tonumber(lua_state, position) );
}

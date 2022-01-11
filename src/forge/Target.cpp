//
// Target.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Target.hpp"
#include "TargetPrototype.hpp"
#include "Graph.hpp"
#include "GraphWriter.hpp"
#include "GraphReader.hpp"
#include "Forge.hpp"
#include "System.hpp"
#include <assert/assert.hpp>
#include <algorithm>
#include <limits>

using std::min;
using std::max;
using std::swap;
using std::remove;
using std::vector;
using std::string;
using std::time_t;
using namespace sweet;
using namespace sweet::forge;

/**
// Constructor.
*/
Target::Target()
: id_(),
  path_(),
  branch_(),
  graph_( NULL ),
  prototype_( NULL ),
  timestamp_( 0 ),
  last_write_time_( 0 ),
  hash_( 0 ),
  pending_hash_( 0 ),
  outdated_( false ),
  changed_( false ),
  bound_to_file_( false ),
  bound_to_dependencies_( false ),
  referenced_by_script_( false ),
  cleanable_( false ),
  built_( false ),
  working_directory_( NULL ),
  parent_( NULL ),
  targets_(),
  dependencies_(),
  implicit_dependencies_(),
  ordering_dependencies_(),
  transitive_dependencies_(),
  filenames_(),
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
: id_( id ),
  path_(),
  branch_(),
  graph_( graph ),
  prototype_( NULL ),
  timestamp_( 0 ),
  last_write_time_( 0 ),
  hash_( 0 ),
  pending_hash_( 0 ),
  outdated_( false ),
  changed_( false ),
  bound_to_file_( false ),
  bound_to_dependencies_( false ),
  referenced_by_script_( false ),
  cleanable_( false ),
  built_( false ),
  working_directory_( NULL ),
  parent_( NULL ),
  targets_(),
  dependencies_(),
  implicit_dependencies_(),
  ordering_dependencies_(),
  filenames_(),
  visiting_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 ),
  postorder_height_( -1 ),
  anonymous_( 0 )
{
    SWEET_ASSERT( !id_.empty() );
    SWEET_ASSERT( graph_ );
}

Target::~Target()
{
    while ( !targets_.empty() )
    {
        delete targets_.back();
        targets_.pop_back();
    }

    if ( graph_ )
    {
        graph_->forge()->destroy_target_lua_binding( this );
    }
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

    for ( vector<Target*>::const_iterator i = targets_.begin(); i != targets_.end(); ++i )
    {
        Target* target = *i;
        SWEET_ASSERT( target );
        target->parent_ = this;
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
const std::string& Target::id() const
{
    return id_;
}

/**
// Get the full path to this Target.
//
// @return
//  The full path to this Target.
*/
const std::string& Target::path() const
{
    if ( path_.empty() )
    {
        path_ = branch() + id();
    }

    return path_;
}

/**
// Get the branch path to this Target is in.
//
// @return
//  The branch path that this target is in.
*/
const std::string& Target::branch() const
{
    if ( branch_.empty() )
    {
        vector<Target*> targets_to_root;

        Target* parent = Target::parent();
        while ( parent )
        {
            targets_to_root.push_back( parent );
            parent = parent->parent();
        }

        if ( !targets_to_root.empty() )
        {
            vector<Target*>::const_reverse_iterator i = targets_to_root.rbegin();
            ++i;

            if ( i != targets_to_root.rend() )
            {
                const char DRIVE = ':';
                if ( (*i)->id().find(DRIVE) != std::string::npos )
                {
                    Target* target = *i;
                    SWEET_ASSERT( target );
                    branch_ += target->id();
                    ++i;
                }
            }
            branch_ += "/";

            while ( i != targets_to_root.rend() )
            {
                Target* target = *i;
                SWEET_ASSERT( target != 0 );
                branch_ += target->id();
                branch_ += "/";
                ++i;
            }
        }
    }

    return branch_;
}

/**
// Is this target an anonymous target?
//
// A target is considered anonymous if its identifier starts with the string
// '$$'.
//
// @return
//  True if this target is anonymous otherwise false.
*/
bool Target::anonymous() const
{
    return id_.size() > 2 && id_[0] == '$' && id_[1] == '$';
}

/**
// Get the 64-bit hash value set for this Target.
//
// @return
//  The hash value or 0 if no hash value was set.
*/
uint64_t Target::hash() const
{
    return hash_;
}

/**
// Get the Graph that this Target is part of.
//
// @return
//  The Graph.
*/
Graph* Target::graph() const
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
void Target::set_prototype( TargetPrototype* target_prototype )
{
    SWEET_ASSERT( !prototype_ || prototype_ == target_prototype );    
    if ( !prototype_ || prototype_ != target_prototype )
    {
        prototype_ = target_prototype;
        if ( referenced_by_script() )
        {
            graph_->forge()->update_target_lua_binding( this );
        }
    }
}

/**
// Get the TargetPrototype for this Target.
//
// @return
//  The TargetPrototype or null if this Target has no TargetPrototype.
*/
TargetPrototype* Target::prototype() const
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
// If the file exists then the timestamp of this Target is set to the last 
// write time of the file so that Targets that depend on this Target will be
// outdated if they are older than this Target.  Additionally if the last 
// write time of the file or directory is different to the last write time 
// already stored in this Target then this Target is marked as having changed.
*/
void Target::bind_to_file()
{
    if ( !bound_to_file_ )
    {
        if ( !filenames_.empty() )
        {
            time_t latest_last_write_time = 0;
            time_t earliest_last_write_time = std::numeric_limits<time_t>::max();
            bool outdated = false;

            for ( vector<string>::const_iterator filename = filenames_.begin(); filename != filenames_.end(); ++filename )
            {
                System* system = graph_->forge()->system();
                if ( system->exists(*filename) )
                {
                    time_t last_write_time = system->last_write_time( *filename );
                    latest_last_write_time = max( last_write_time, latest_last_write_time );
                    earliest_last_write_time = min( last_write_time, earliest_last_write_time );
                }
                else
                {
                    latest_last_write_time = std::numeric_limits<time_t>::max();
                    earliest_last_write_time = 0;
                    outdated = true;
                }
            }

            changed_ = last_write_time_ != earliest_last_write_time;
            timestamp_ = latest_last_write_time;
            last_write_time_ = earliest_last_write_time;
            outdated_ = outdated || hash_ != pending_hash_;
            hash_ = pending_hash_;
        }
        else
        {
            changed_ = last_write_time_ != 0;
            timestamp_ = 0;
            last_write_time_ = 0;
            outdated_ = !built_ || hash_ != pending_hash_;
            hash_ = pending_hash_;
        }
        
        bound_to_file_ = true;
    }
}

/**
// Bind this Target to its dependencies.
//
// Sets the timestamp of this Target to be the latest last write time of the 
// files that it is bound to or the latest timestamp of any of its 
// dependencies.
// 
// If this Target is bound to one or more files then it is outdated if any of
// its dependencies have a timestamp later than its last write time.  If this
// Target is not bound to any files then it is outdated if any of its
// dependencies are outdated.
//
// Cleanable Targets that haven't been built are always outdated whether they
// are bound to files or not.
*/
void Target::bind_to_dependencies()
{
    if ( !bound_to_dependencies_ )
    {
        time_t timestamp = timestamp_;
        bool outdated = outdated_;
        int finish = int(dependencies_.size() + implicit_dependencies_.size());

        for (int i = 0; i < finish; ++i)
        {
            Target* target = any_dependency( i );
            SWEET_ASSERT( target );
            outdated = outdated || target->outdated();
            timestamp = std::max( timestamp, target->timestamp() );
        }

        if ( !filenames_.empty() )
        {
            outdated = outdated_ || timestamp > last_write_time();
        }

        outdated = outdated || (cleanable_ && !built_);

        set_outdated( outdated );
        set_timestamp( timestamp );
        bound_to_dependencies_ = true;
    }
}

/**
// Set the settings hash for this Target.
//
// @param hash
//  The hash value to set for this Target.
*/
void Target::set_hash( uint64_t hash )
{
    pending_hash_ = hash;
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
bool Target::referenced_by_script() const
{
    return referenced_by_script_;
}

/**
// Set whether or not this Target is able to be cleaned.
//
// @param cleanable
//  True to set this Target as able to be cleaned or otherwise false to 
//  prevent this Target from being cleaned.
*/
void Target::set_cleanable( bool cleanable )
{
    cleanable_ = cleanable;
}

/**
// Is this Target able to be cleaned?
//
// @return
//  True if this Target is able to be cleaned otherwise false.
*/
bool Target::cleanable() const
{
    return cleanable_;
}

/**
// Set whether or not this Target is considered to have been built at least 
// once.
//
// @param built
//  True to mark this Target has having been built otherwise false to indicate
//  that this Target should be considered outdated if it is also cleanable.
*/
void Target::set_built( bool built )
{
    built_ = built;
}

/**
// Has this Target been built at least once?
//
// @return
//  True if this Target has been built at least once otherwise false.
*/
bool Target::built() const
{
    return built_;
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
std::time_t Target::timestamp() const
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
std::time_t Target::last_write_time() const
{
    return last_write_time_;
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
bool Target::outdated() const
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
bool Target::changed() const
{
    return changed_;
}

/**
// Is this Target bound?
//
// @return
//  True if this Target is bound otherwise false.
*/
bool Target::bound_to_file() const
{
    return bound_to_file_;
}

/**
// Add another filename to the filenames that this Target is bound to.
//
// @param filename
//  The name of the file to add to the filenames that this target is bound
//  to (assumed not empty).
*/
void Target::add_filename( const std::string& filename )
{
    filenames_.push_back( filename );
}

/**
// Set the filename that this Target is bound to.
//
// If \e index is out of the current range of filenames then empty filenames
// are inserted up to and including the filename at \e index.
//
// @param filename
//  The name of the file to bind this Target to or an empty string to set 
//  this Target not to be bound to any file.
//
// @param index
//  The index of the filename to set.
*/
void Target::set_filename( const std::string& filename, int index )
{
    SWEET_ASSERT( index >= 0 );
    if ( index >= int(filenames_.size()) )
    {
        filenames_.insert( filenames_.end(), index - filenames_.size() + 1, string() );
    }
    filenames_[index] = filename;
}

/**
// Clear the filenames that this Target is bound to.
*/
void Target::clear_filenames( int start, int finish )
{
    vector<string>::iterator begin = filenames_.begin() + max( start, 0 );
    vector<string>::iterator end = filenames_.begin() + min( finish, int(filenames_.size()) );
    filenames_.erase( begin, end );
}

/**
// Get the filename that this Target is bound to.
//
// @param n
//  The index of the filename to return (assumed to be within the valid range
//  of filenames added to this Target; [0, #filenames)).
//
// @return
//  The nth filename that this Target is bound to.
*/
const std::string& Target::filename( int n ) const
{
    SWEET_ASSERT( n >= 0 && n < int(filenames_.size()) );
    return filenames_[n];
}

/**
// Get the filenames bound to this Target.
//
// @return
//  The filenames.
*/
const std::vector<std::string>& Target::filenames() const
{
    return filenames_;
}

/**
// Get the directory portion of the \e nth filename that this Target is bound 
// to.
//
// @param n
//  The index of the filename to return the directory portion of (assumed to 
//  be within that valid range of filenames added to this Target; [0, 
//  #filenames)).
//
// @return
//  The directory portion of the nth filename that this Target is bound to.
*/
std::string Target::directory( int n ) const
{
    SWEET_ASSERT( n >= 0 && n < int(filenames_.size()) );
    return boost::filesystem::path( filenames_[n] ).parent_path().generic_string();
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
void Target::set_working_directory( Target* target )
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
Target* Target::working_directory() const
{
    return working_directory_;
}

/**
// Set the Target that is a parent of this Target in the Target namespace.
//
// @param target
//  The Target to make a parent of this Target.
*/
void Target::set_parent( Target* target )
{
    parent_ = target;
}

/**
// Get the Target that is a parent of this Target.
//
// @return
//  The Target that is a parent of this Target or null if this Target is the
//  root Target.
*/
Target* Target::parent() const
{
    return parent_;
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
void Target::add_target( Target* target, Target* this_target )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( this_target == this );
    SWEET_ASSERT( std::find(targets_.begin(), targets_.end(), target) == targets_.end() );
    SWEET_ASSERT( target->id().empty() || !find_target_by_id(target->id()) );
    SWEET_ASSERT( !target->parent() );

    targets_.push_back( target );
    target->set_parent( this_target );
}

/**
// Destroy any anonymous targets that are direct children of this target.
*/
void Target::destroy_anonymous_targets()
{
    for ( vector<Target*>::iterator i = targets_.begin(); i != targets_.end(); ++i )
    {
        Target* target = *i;
        if ( target->anonymous() )
        {
            delete target;
            *i = NULL;
        }
    }
    targets_.erase( remove(targets_.begin(), targets_.end(), (Target*) NULL), targets_.end() );
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
Target* Target::find_target_by_id( const std::string& id ) const
{
    vector<Target*>::const_iterator i = targets_.begin();
    while ( i != targets_.end() && (*i)->id() != id )
    {
        ++i;
    }
    return i != targets_.end() ? *i : NULL;
}

/**
// Get the Targets that are part of this Target.
//
// @return
//  The Targets.
*/
const std::vector<Target*>& Target::targets() const
{
    return targets_;
}

/**
// Add a Target as an explicit dependency of this Target.
//
// If \e target is null or is already an explicit dependency of this Target 
// then this function silently does nothing.  This allows calling code to be 
// simpler in that optional dependencies that are not enabled can be 
// implemented by setting the optional dependency Target to null and it saves
// calling code from having to make sure that it doesn't add dependencies 
// twice.
//
// Dependencies that create cycles in the Graph aren't checked for by this 
// function but are detected and reported during Graph traversal.  In these
// cases the graph traversal continues as if the cyclic dependency was built 
// successfully when it is recursively visited.
//
// If a dependency is removed then the the bound to dependencies flag for this
// Target is cleared to indicate that the outdated flag and/or timestamp are
// potentially invalid.
//
// @param target
//  The Target to add as a dependency.
*/
void Target::add_explicit_dependency( Target* target )
{
    bool able_to_add_explicit_dependency = 
        target && 
        target != this && 
        !is_explicit_dependency( target )
    ;
    if ( able_to_add_explicit_dependency )
    {
        SWEET_ASSERT( target->graph() == graph() );
        remove_dependency( target );
        dependencies_.push_back( target );
        bound_to_dependencies_ = false;
    }
}

/**
// Remove all explicit dependencies from this Target.
//
// The bound to dependencies flag for this Target is cleared to indicate that
// the outdated flag and/or timestamp are potentially invalid.
*/
void Target::clear_explicit_dependencies()
{
    dependencies_.clear();
    bound_to_dependencies_ = false;
}

/**
// Add 'target' as an implicit dependency of this Target.
//
// If a dependency is added then the the bound to dependencies flag for this
// Target is cleared to indicate that the outdated flag and/or timestamp are
// potentially invalid.
//
// @param target
//  The Target to add as an implicit dependency (quietly ignored if null,
//  the same as this target, an anonymous target, or already a dependency).
*/
void Target::add_implicit_dependency( Target* target )
{
    bool able_to_add_implicit_dependency =
        target &&
        target != this &&
        !target->anonymous() &&
        !is_explicit_dependency( target ) &&
        !is_implicit_dependency( target )
    ;
    if ( able_to_add_implicit_dependency )
    {
        remove_dependency( target );
        implicit_dependencies_.push_back( target );
        bound_to_dependencies_ = false;
    }
}

/**
// Remove 'target' from the implicit dependencies of this Target.
//
// If a dependency is removed then the the bound to dependencies flag for this
// Target is cleared to indicate that the outdated flag and/or timestamp are
// potentially invalid.
//
// @param target
//  The Target to remove from this Target's implicit dependencies.
*/
void Target::remove_implicit_dependency( Target* target )
{
    if ( target && target != this )
    {
        SWEET_ASSERT( target->graph() == graph() );
        vector<Target*>::iterator i = find( implicit_dependencies_.begin(), implicit_dependencies_.end(), target );
        if ( i != implicit_dependencies_.end() )
        {
            implicit_dependencies_.erase( i );
            bound_to_dependencies_ = false;
        }
    }
}

/**
// Clear this Target's implicit dependencies.
//
// The bound to dependencies flag for this Target is cleared to indicate that
// the outdated flag and/or timestamp are potentially invalid.
*/
void Target::clear_implicit_dependencies()
{
    implicit_dependencies_.clear();
    bound_to_dependencies_ = false;
}

/**
// Add an ordering dependency to this Target.
//
// @param target
//  The Target to add as an ordering dependency of this Target.
*/
void Target::add_ordering_dependency( Target* target )
{
    bool able_to_add_ordering_dependency = 
        target &&
        target != this &&
        !is_dependency( target )
    ;
    if ( able_to_add_ordering_dependency )
    {
        remove_dependency( target );
        ordering_dependencies_.push_back( target );
    }
}

/**
// Clear this Target's ordering dependencies.
*/
void Target::clear_ordering_dependencies()
{
    ordering_dependencies_.clear();
}

void Target::add_transitive_dependency( Target* target )
{
    bool able_to_add = target && target != this && !is_dependency( target );
    if ( able_to_add )
    {
        remove_dependency( target );
        transitive_dependencies_.push_back( target );
    }
}

void Target::clear_transitive_dependencies()
{
    transitive_dependencies_.clear();
}

/**
// Remove a dependency of this Target.
//
// If \e target is null or is not a dependency of this Target then this 
// function silently does nothing.
//
// Targets are removed from both this Target's explicit, implicit, and 
// ordering dependencies.
//
// If an explicit or implicit dependency is removed then the the bound to 
// dependencies flag for this Target is cleared to indicate that the outdated
// flag and/or timestamp are potentially invalid.
//
// @param target
//  The Target to remove as a dependency.
*/
void Target::remove_dependency( Target* target )
{
    if ( target && target != this )
    {
        SWEET_ASSERT( target->graph() == graph() );
        vector<Target*>::iterator i = find( dependencies_.begin(), dependencies_.end(), target );
        if ( i != dependencies_.end() )
        {
            dependencies_.erase( i );
            bound_to_dependencies_ = false;
        }
        else 
        {
            i = find( implicit_dependencies_.begin(), implicit_dependencies_.end(), target );
            if ( i != implicit_dependencies_.end() )
            {
                implicit_dependencies_.erase( i );
                bound_to_dependencies_ = false;
            }
            else 
            {
                i = find( ordering_dependencies_.begin(), ordering_dependencies_.end(), target );
                if ( i != ordering_dependencies_.end() )
                {
                    ordering_dependencies_.erase( i );
                }
            }
        }
    }
}

/**
// Is *target* an explicit dependency of this Target?
//
// @param target
//  The Target to check for being an explicit dependency of this Target.
//
// @return
//  True if *target* is an explicit dependency of this Target otherwise false.
*/
bool Target::is_explicit_dependency( Target* target ) const
{
    return find( dependencies_.begin(), dependencies_.end(), target ) != dependencies_.end();
}

/**
// Is *target* an implicit dependency of this Target?
//
// @param target
//  The Target to check for being an implicit dependency of this Target.
//
// @return
//  True if *target* is an implicit dependency of this Target otherwise false.
*/
bool Target::is_implicit_dependency( Target* target ) const
{
    return find( implicit_dependencies_.begin(), implicit_dependencies_.end(), target ) != implicit_dependencies_.end();
}

/**
// Is *target* an ordering dependency of this Target?
//
// @param target
//  The Target to check for being an ordering dependency of this Target.
//
// @return
//  True if *target* is an ordering dependency of this Target otherwise false.
*/
bool Target::is_ordering_dependency( Target* target ) const
{
    return find( ordering_dependencies_.begin(), ordering_dependencies_.end(), target ) != ordering_dependencies_.end();
}

bool Target::is_transitive_dependency( Target* target ) const
{
    return find( transitive_dependencies_.begin(), transitive_dependencies_.end(), target ) != transitive_dependencies_.end();
}

/**
// Is a Target a dependency of this Target?
//
// @param target
//  The Target to check for being a dependency of this Target.
//
// @return
//  True if 'target' is an explicit or implicit dependency of this Target 
//  otherwise false.
*/
bool Target::is_dependency( Target* target ) const
{
    return 
        is_explicit_dependency( target ) ||
        is_implicit_dependency( target ) ||
        is_ordering_dependency( target ) ||
        is_transitive_dependency( target )
    ;
}

/**
// Get the nth explicit dependency of this Target.
//
// @param n
//  The index of the explicit dependency to return (assumed to be > 0).
//
// @return
//  The \e nth explicit dependency of this Target or null if 'n' is outside 
//  the range of explicit dependencies.
*/
Target* Target::explicit_dependency( int n ) const
{
    SWEET_ASSERT( n >= 0 );
    if ( n >= 0 && n < int(dependencies_.size()) )
    {
        return dependencies_[n];
    }
    return NULL;
}

/**
// Get the nth implicit dependency of this Target.
//
// @param n
//  The index of the implicit dependency to return (assumed to be > 0).
//
// @return
//  The \e nth implicit dependency of this Target or null if 'n' is outside 
//  the range of implicit dependencies.
*/
Target* Target::implicit_dependency( int n ) const
{
    SWEET_ASSERT( n >= 0 );
    if ( n >= 0 && n < int(implicit_dependencies_.size()) )
    {
        return implicit_dependencies_[n];
    }
    return NULL;
}

/**
// Get the nth ordering dependency of this Target.
//
// @param n
//  The index of the ordering dependency to return (assumed >= 0).
//
// @return
//  The nth ordering dependency of this Target or null if 'n' is outside the
//  range of ordering dependencies.
*/
Target* Target::ordering_dependency( int n ) const
{
    SWEET_ASSERT( n >= 0 );
    if ( n >= 0 && n < int(ordering_dependencies_.size()) )
    {
        return ordering_dependencies_[n];
    }
    return NULL;
}

/**
// Get the nth transitive dependency of this Target.
//
// @param n
//  The index of the transitive dependency to return (assumed >= 0).
//
// @return
//  The nth transitive dependency of this Target or null if 'n' is outside the
//  range of transitive dependencies.
*/
Target* Target::transitive_dependency( int n ) const
{
    SWEET_ASSERT( n >= 0 );
    if ( n >= 0 && n < int(transitive_dependencies_.size()) )
    {
        return transitive_dependencies_[n];
    }
    return nullptr;
}

/**
// Get the 'nth' dependency of any kind from this Target.
//
// The index is resolved into the explicit, implicit, and ordering 
// dependencies of this Target by treating all of the dependencies to be in a
// single array in the order of explicit, implicit, and then ordering 
// dependencies.
//
// A value of 'n' in the range [0, #explicit) selects an explicit dependency, 
// a value of 'n' in the range [#explicit, #explicit + #implicit) selects an 
// implicit dependency, and similarly a value of 'n' in the range 
// [#explicit + #implicit, #explicit + #implicit + #ordering) selects an 
// ordering dependency.
//
// Any value of 'n' outside of those ranges selects null.
//
// @param n
//  The index of the dependency to return (assumed to be > 0).
//
// @return
//  The \e nth dependency of this Target or null if 'n' is outside the range
//  of valid dependencies.
*/
Target* Target::any_dependency( int n ) const
{
    SWEET_ASSERT( n >= 0 );

    if ( n >= 0 && n < int(dependencies_.size()) )
    {
        return dependencies_[n];
    }

    n -= int(dependencies_.size());
    if ( n >= 0 && n < int(implicit_dependencies_.size()) )
    {
        return implicit_dependencies_[n];
    }

    n -= int(implicit_dependencies_.size());
    if ( n >= 0 && n < int(ordering_dependencies_.size()) )
    {
        return ordering_dependencies_[n];
    }

    n -= int(ordering_dependencies_.size());
    if ( n >= 0 && n < int(transitive_dependencies_.size()) )
    {
        return transitive_dependencies_[n];
    }

    return nullptr;
}

/**
// Are all of the dependencies of this Target built successfully?
//
// @return
//  True if all of this Target's dependencies were built successfully.
*/
bool Target::buildable() const
{
    int i = 0;
    Target* target = any_dependency( i );
    while ( target && target->successful() ) 
    {
        ++i;
        target = any_dependency( i );
    }
    return target == NULL;
}

/**
// Generate a string containing the prototype of this Target and its full 
// path for use in error reporting.
//
// @return
//  A string that identifies this Target for error reporting.
*/
std::string Target::error_identifier() const
{
    char buffer [1024];
    if ( prototype_ )
    {
        snprintf( buffer, sizeof(buffer), "%s '%s'", prototype_->id().c_str(), path().c_str() );
        buffer [sizeof(buffer) - 1] = 0;
        return string( buffer );
    }
    else
    {
        snprintf( buffer, sizeof(buffer), "'%s'", path().c_str() );
        buffer [sizeof(buffer) - 1] = 0;
        return string( buffer );
    }
}

/**
// Generate a string containing the dependencies of this Target that failed 
// in the current traversal.
//
// @return
//  A string containing the failed dependencies.
*/
std::string Target::failed_dependencies() const
{
    SWEET_ASSERT( !buildable() );

//
// Append the identifier for this Target.
//
    std::string message;
    if ( !id().empty() )
    {
        message += "'" + id() + "'";
    }
    else if ( prototype() != NULL )
    {
        message += prototype()->id();
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
    int i = 0; 
    Target* target = any_dependency( i );
    while ( target && target->successful() )
    {
        ++i;
        target = any_dependency( i );
    }
    
    SWEET_ASSERT( target );
    if ( target )
    {
        if ( !target->id().empty() )
        {
            message += "'" + target->id() + "'";
        }
        else if ( target->prototype() )
        {
            message += target->prototype()->id();
        }
        else
        {
            message += "<anonymous>";
        }

        ++i;
        target = any_dependency( i );
    }

//
// Append the remaining failed dependencies.
//
    while ( target )
    {       
        if ( !target->successful() )
        {
            if ( !target->id().empty() )
            {
                message += ", '" + target->id() + "'";
            }
            else if ( target->prototype() != 0 )
            {
                message += ", " + target->prototype()->id();
            }
            else
            {
                message += ", <anonymous>";
            }
        }

        ++i;
        target = any_dependency( i );
    }

    return message;
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
bool Target::visiting() const
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
    visited_revision_ = visited ? graph_->visited_revision() : graph_->visited_revision() - 1;
}

/**
// Is this Target marked as having been visited in the current pass?
//
// @return
//  True if this Target has been visited in the current pass otherwise false.
*/
bool Target::visited() const
{
    SWEET_ASSERT( graph_ );
    return visited_revision_ == graph_->visited_revision();
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
    successful_revision_ = successful ? graph_->successful_revision() : graph_->successful_revision() - 1;
}

/**
// Has this Target been visited successfully in the current pass?
//
// @return
//  True if this Target has been visited successfully in the current pass.
*/
bool Target::successful() const
{
    SWEET_ASSERT( graph_ );
    return successful_revision_ == graph_->successful_revision();
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
int Target::next_anonymous_index()
{
    return anonymous_++;
}

/**
// Write this Target to \e writer.
//
// @param writer
//  The GraphWriter to use to serialize this Target to.
*/
void Target::write( GraphWriter& writer )
{
    writer.object_address( this );
    writer.value( id_ );
    writer.value( last_write_time_ );
    writer.value( hash_ );
    writer.value( built_ );
    writer.value( filenames_ );
    writer.value( targets_ );
    writer.refer( implicit_dependencies_ );
}

/**
// Read this Target from \e reader.
//
// @param reader 
//  The GraphReader to deserialize this Target from.
*/
void Target::read( GraphReader& reader )
{
    reader.object_address( this );
    reader.value( &id_ );
    reader.value( &last_write_time_ );
    reader.value( &hash_ );
    reader.value( &built_ );
    reader.value( &filenames_ );
    reader.value( &targets_ );
    reader.refer( &implicit_dependencies_ );
}

/**
// Resolve this Target's implicit dependencies to their new addresses.
//
// Recursively updates the addresses of this Target and its ancestors implicit
// dependencies from their old addresses when the Graph was written out to 
// their new addresses now that the Graph has been read back in.
//
// @param reader
//  The GraphReader just read in the Graph.
*/
void Target::resolve( const GraphReader& reader )
{
    for ( vector<Target*>::iterator i = implicit_dependencies_.begin(); i != implicit_dependencies_.end(); ++i )
    {
        *i = reinterpret_cast<Target*>( reader.find_address_by_old_address(*i) );
    }
    implicit_dependencies_.erase( remove(implicit_dependencies_.begin(), implicit_dependencies_.end(), nullptr), implicit_dependencies_.end() );

    for ( vector<Target*>::const_iterator i = targets_.begin(); i != targets_.end(); ++i )
    {
        Target* target = *i;
        SWEET_ASSERT( target );
        target->resolve( reader );
    }
}

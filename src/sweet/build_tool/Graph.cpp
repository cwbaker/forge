//
// Graph.cpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Graph.hpp"
#include "Error.hpp"
#include "TargetPrototype.hpp"
#include "Target.hpp"
#include "BuildTool.hpp"
#include "ScriptInterface.hpp"
#include "Scheduler.hpp"
#include "OsInterface.hpp"
#include "persist.hpp"
#include <sweet/assert/assert.hpp>

using std::list;
using std::vector;
using std::string;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

namespace sweet
{

namespace build_tool
{

struct Deleter
{
    ScriptInterface* script_interface_;
    
    public:
        Deleter( ScriptInterface* script_interface )
        : script_interface_( script_interface )
        {
            SWEET_ASSERT( script_interface_ );
        }
        
        void operator()( Target* target ) const
        {
            SWEET_ASSERT( target );
            if ( target->is_referenced_by_script() )
            {
                script_interface_->destroy_target( target );
            }
            delete target;
        }
};

}

}

/**
// Constructor.
*/
Graph::Graph()
: build_tool_( NULL ),
  filename_(),
  root_target_(),
  cache_target_(),
  implicit_dependencies_( false ),
  traversal_in_progress_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 )
{
}

/**
// Constructor.
//
// @param build_tool
//  The BuildTool that this Graph is part of.
*/
Graph::Graph( BuildTool* build_tool )
: build_tool_( build_tool ),
  filename_(),
  root_target_(),
  cache_target_(),
  implicit_dependencies_( false ),
  traversal_in_progress_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 )
{
    SWEET_ASSERT( build_tool_ );
    root_target_.reset( new Target("", this) );
}

/**
// Get the root Target.
//
// @return
//  The root Target.
*/
Target* Graph::get_root_target() const
{
    return root_target_.get();
}

/**
// Get the Target for the file that this Graph is loaded from.
//
// @return
//  The Target for this file that this Graph is loaded from or null if this
//  Graph was loaded from buildfiles and not a cache file.
*/
Target* Graph::get_cache_target() const
{
    return cache_target_.get();
}
        
/**
// Get the BuildTool that this Graph is part of.
//
// @return
//  The BuildTool.
*/
BuildTool* Graph::get_build_tool() const
{
    SWEET_ASSERT( build_tool_ );
    return build_tool_;
}

/**
// Mark this graph as being traversed and increment the visited and 
// successful revisions.
//
// The graph is marked as being traversed so that recursive calls to preorder
// or postorder can be detected and report errors.
//
// The visited and successful revisions are incremented so that any Targets 
// that were previously visited now have different visit and success 
// revisions and will be considered not visited and not successful.
*/
void Graph::begin_traversal()
{
    SWEET_ASSERT( !traversal_in_progress_ );
    traversal_in_progress_ = true;
    ++visited_revision_;
    ++successful_revision_;
}

/**
// Mark this graph as not being traversed.
*/
void Graph::end_traversal()
{
    SWEET_ASSERT( traversal_in_progress_ );
    traversal_in_progress_ = false;
}

/**
// Is this graph being traversed?
//
// @return
//  True if this graph is being traversed otherwise false.
*/
bool Graph::is_traversal_in_progress() const
{
    return traversal_in_progress_;
}

/**
// Get the current visited revision for this Graph.
//
// This is used to determine whether or not Targets have been visited during
// a pass.  At the begining of the pass Graph::increment_revisions() 
// is called to increment the visited and successful revisions for the pass.  
// Then when a Target is visited its visited revision is set to be the same 
// as the visited revision in the Graph.  A Target can then be checked to see 
// if it has been visited by checking if its visited revision is the same as 
// the current visited revision for its Graph.
//
// @return
//  The current visited revision for this Graph.
*/
int Graph::get_visited_revision() const
{
    return visited_revision_;
}

/**
// Get the current success revision for this Graph.
//
// This is used to determine whether or not Targets have been visited 
// successfully during a pass.  See Graph::get_visit_revision() for a
// description of how the revision values are used.
//
// @return
//  The current successful revision for this Graph.
*/
int Graph::get_successful_revision() const
{
    return successful_revision_;
}

/**
// Find or create a Target.
//
// Finds or create the Target by breaking \e id into '/' delimited elements
// and searching up or down the target hierarchy relative to 
// \e working_directory or the root directory.  
//
// If a ".." element is encountered then the relative parent is moved up a 
// level otherwise a new Target is added with that identifier as a child of 
// the current relative parent and the next element is considered.
//
// If \e id refers to an already existing Target that doesn't have a 
// TargetPrototype set then the existing Target is updated to have 
// \e target_prototype as its TargetPrototype and \e working_directory as its
// working directory.  The working directory is updated to allow Targets for 
// C/C++ libraries to be lazily created as plain file Targets by depending 
// executables before the libraries themselves are defined.  When the library
// is lazily defined by the executable reference the working directory is the
// working directory for the executable.  When the library is defined it 
// specifies the correct TargetPrototype and working directory.
//
// If \e id refers to an already existing Target that already has a 
// TargetPrototype set then an error is generated as it is not clear which
// TargetPrototype applies.
//
// @param id
//  The identifier of the Target to find or create.
//
// @param target_prototype
//  The TargetPrototype of the Target to create or null to create a Target that has no
//  TargetPrototype.
//
// @param working_directory
//  The Target that the identifier is relative to or null if the identifier
//  is relative to the root Target.
//
// @return
//  The Target.
*/
ptr<Target> Graph::target( const std::string& id, ptr<TargetPrototype> target_prototype, ptr<Target> working_directory )
{
    path::Path path( id );
    ptr<Target> target( working_directory && path.is_relative() ? working_directory : root_target_ );
    SWEET_ASSERT( target );

    if ( !id.empty() )
    {
        path::Path::const_iterator i = path.begin();
        while ( i != path.end() )
        {
            const string& element = *i;
            ++i;
                   
            if ( element == "." )
            {
                target = target;
            }
            else if ( element == ".." )
            {
                SWEET_ASSERT( target );
                SWEET_ASSERT( target->get_parent() );
                SWEET_ASSERT( target->get_parent()->get_graph() == this );
                target = target->get_parent();
            }
            else
            {
                ptr<Target> new_target = target->find_target_by_id( element );
                if ( !new_target )
                {
                    new_target.reset( new Target(element, this), Deleter(build_tool_->get_script_interface()) );
                    target->add_target( new_target, target );
                    if ( i != path.end() )
                    {
                        new_target->set_working_directory( target );
                    }
                }

                target = new_target;
            }
        }
    }
    else
    {
        ptr<Target> new_target( new Target("", this), Deleter(build_tool_->get_script_interface()) );
        target->add_target( new_target, target );
        target = new_target;
    }

    if ( target_prototype && target->get_prototype() == NULL )
    {
        target->set_prototype( target_prototype );
        target->set_working_directory( working_directory );
    }

    if ( target->get_working_directory() == NULL )
    {
        target->set_working_directory( working_directory );
    }

    if ( target_prototype && target->get_prototype() != target_prototype )
    {
        SWEET_ERROR( PrototypeConflictError("The target '%s' has been created with prototypes '%s' and '%s'", id.c_str(), target->get_prototype()->get_id().c_str(), target_prototype ? target_prototype->get_id().c_str() : "none" ) );
    }

    return target;
}

/**
// Find a Target in this Graph.
//
// Find the Target by breaking the id into '/' delimited elements and 
// searching up or down the Target hierarchy relative to 
// \e working_directory.
//
// @param id
//  The id of the Target to find.
//
// @param working_directory
//  The Target that the identifier is relative to or null if the identifier
//  is relative to the root Target.
//
// @return
//  The Target or null if no matching Target was found.
*/
ptr<Target> Graph::find_target( const std::string& id, ptr<Target> working_directory )
{
    ptr<Target> target;

    if ( !id.empty() )
    {
        path::Path path( id );
        target = working_directory && path.is_relative() ? working_directory : root_target_;
        SWEET_ASSERT( target );

        path::Path::const_iterator i = path.begin();
        while ( i != path.end() && target )
        {       
            if ( *i == "." )
            {
                target = target;
            }
            else if ( *i == ".." )
            {
                target = target->get_parent();
            }
            else
            {
                target = target->find_target_by_id( *i );
            }

            ++i;
        }    
    }

    return target;
}

/**
// Load a buildfile into this Graph.
//
// @param filename
//  The name of the buildfile to load.
//
// @param target
//  The Target that top level Targets in the buildfile are made dependencies 
//  of or null to make top level Targets dependencies of the target that 
//  corresponds to the working directory.
*/
void Graph::buildfile( const std::string& filename, ptr<Target> target )
{
    SWEET_ASSERT( build_tool_ );
    SWEET_ASSERT( root_target_ );
     
    path::Path path( build_tool_->get_script_interface()->absolute(filename) );
    SWEET_ASSERT( path.is_absolute() );
    
    ptr<Target> buildfile_target = Graph::target( path.string(), ptr<TargetPrototype>(), ptr<Target>() );
    buildfile_target->set_filename( path.string() );
    if ( cache_target_ )
    {
        cache_target_->add_dependency( buildfile_target );
    }
    build_tool_->get_scheduler()->buildfile( path );
}

struct ScopedVisit
{
    Target* target_;

    ScopedVisit( Target* target )
    : target_( target )
    {
        SWEET_ASSERT( target_ );
        SWEET_ASSERT( target_->is_visiting() == false );
        target_->set_visited( true );
        target_->set_visiting( true );
    }

    ~ScopedVisit()
    {
        SWEET_ASSERT( target_ );
        SWEET_ASSERT( target_->is_visiting() == true );
        target_->set_visiting( false );
    }
};

struct Bind
{
    BuildTool* build_tool_;
    int failures_;
    
    Bind( BuildTool* build_tool )
    : build_tool_( build_tool ),
      failures_( 0 )
    {
        SWEET_ASSERT( build_tool_ );
        build_tool_->get_graph()->begin_traversal();
    }
    
    ~Bind()
    {
        build_tool_->get_graph()->end_traversal();
    }

    void visit( Target* target )
    {
        SWEET_ASSERT( target );

        if ( !target->is_visited() )
        {
            ScopedVisit visit( target );

            const vector<Target*>& dependencies = target->get_dependencies();
            for ( vector<Target*>::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i )
            {
                Target* dependency = *i;
                SWEET_ASSERT( dependency );

                if ( !dependency->is_visiting() )
                {
                    Bind::visit( dependency );
                }
                else
                {
                    build_tool_->warning( "Ignoring cyclic dependency from '%s' to '%s' during binding", target->get_id().c_str(), dependency->get_id().c_str() );
                    dependency->set_successful( true );
                }
            }

            target->bind();
            target->set_successful( true );

            if ( target->is_required_to_exist() && target->is_bound_to_file() && target->get_last_write_time() == 0 )
            {
                build_tool_->error( "The source file '%s' does not exist", target->get_filename().c_str() );
                ++failures_;
            }
        }
    }
};

/**
// Make a postorder pass over this Graph to bind its Targets.
//
// @param target
//  The Target to begin the visit at or null to begin the visitation from 
//  the root of the Graph.
//
// @return
//  The number of Targets that failed to bind because they were files that
//  were expected to exist (see Target::set_required_to_exist()).
*/
int Graph::bind( ptr<Target> target )
{
    SWEET_ASSERT( !target || target->get_graph() == this );

    Bind bind( build_tool_ );
    bind.visit( target ? target.get() : root_target_.get() );
    return bind.failures_;
}

/**
// Swap this Graph with \e graph.
//
// @param graph
//  The Graph to swap this Graph's members with.
*/
void Graph::swap( Graph& graph )
{
    std::swap( implicit_dependencies_, graph.implicit_dependencies_ );
    root_target_.swap( graph.root_target_ );
}

/**
// Clear all of the targets in this graph.
//
// The name of the file that this Graph was most recently loaded from is used
// to create the implicit cache target in the newly cleared Graph.  This gives
// an implicit target for the cache file that buildfiles are able to depend 
// on (see Graph::buildfile()) so that the cache can be marked as outdated if
// any buildfiles change and become newer than the cache.
//
// This might be a bit strange if the Graph is then saved to a cache file with
// a different filename which is why the save functions don't take filename
// arguments and one of the load functions must be called first to provide the
// name of the cache file.
*/
void Graph::clear()
{
    SWEET_ASSERT( build_tool_ );

    root_target_.reset( new Target("", this) );
    cache_target_.reset();
    recover();
    implicit_dependencies_ = false;
}

/**
// Recover implicit relationships after this Graph has been loaded from an
// Archive.
*/
void Graph::recover()
{
    SWEET_ASSERT( root_target_ );
    root_target_->recover( this );
    if ( !filename_.empty() )
    {
        cache_target_ = target( filename_, ptr<TargetPrototype>(), ptr<Target>() );
        cache_target_->set_filename( filename_ );
        bind( cache_target_ );
    }
}

/**
// Mark added dependencies as being implicit.
//
// Dependencies are added as explicit dependencies from when a Graph is 
// initially created or cleared (see Graph::clear()) until the 
// Graph::mark_implicit_dependencies() is called.  After 
// Graph::mark_implicit_dependencies() has been called dependencies are added
// as implicit dependencies.
//
// Implicit dependencies are cleared whenever the Target that depends on them 
// is scanned -- they are intended to represent the implicit dependency 
// relationships discovered while scanning source files (e.g. when scanning 
// C/C++ source files for the header files that they include).
//
// Explicit dependencies are not cleared when the Target that depends on them 
// is scanned -- they are intended to represent the explicit dependency 
// relationships expressed directly in the buildfiles loaded to specify the 
// initial dependency graph.
*/
void Graph::mark_implicit_dependencies()
{
    implicit_dependencies_ = true;
}

/**
// Are dependencies added to Targets implicit?
//
// @return
//  True if added dependencies are implicit dependencies otherwise false to 
//  indicate that added dependencies are explicit.
*/
bool Graph::implicit_dependencies() const
{
    return implicit_dependencies_;
}

/**
// Load this Graph from a binary file.
//
// @param filename
//  The name of the file to load this Graph from.
//
// @return
//  The target that corresponds to the file that this Graph was loaded from or
//  null if there was no cache target.
*/
ptr<Target> Graph::load_xml( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( build_tool_ );

    filename_ = filename;
    cache_target_.reset();

    if ( build_tool_->get_os_interface()->exists(filename) )
    {
        path::Path path( filename );
        SWEET_ASSERT( path.is_absolute() );
        sweet::persist::XmlReader reader( build_tool_->error_policy() );
        enter( reader );
        Graph graph;
        reader.read( path.string().c_str(), "graph", graph );
        exit( reader );
        swap( graph );
        recover();
    }
    return cache_target_;
}

/**
// Save this Graph to an XML file.
//
// @param filename
//  The name of the file to save this Graph to.
*/
void Graph::save_xml()
{
    SWEET_ASSERT( build_tool_ );

    if ( !filename_.empty() )
    {
        path::Path path( filename_ );
        sweet::persist::XmlWriter writer( build_tool_->error_policy() );
        enter( writer );
        writer.write( path.string().c_str(), "graph", *this );
        exit( writer );
    }
    else
    {
        build_tool_->error( "Unable to save a dependency graph without trying to load it first" );
    }
}

/**
// Load this Graph from a binary file.
//
// @param filename
//  The name of the file to load this Graph from.
//
// @return
//  The target that corresponds to the file that this Graph was loaded from or
//  null if there was no cache target.
*/
ptr<Target> Graph::load_binary( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( build_tool_ );
    
    filename_ = filename;
    cache_target_.reset();

    if ( build_tool_->get_os_interface()->exists(filename) )
    {
        path::Path path( filename );
        SWEET_ASSERT( path.is_absolute() );
        sweet::persist::BinaryReader reader( build_tool_->error_policy() );
        enter( reader );
        Graph graph;
        reader.read( path.string().c_str(), "graph", graph );
        exit( reader );
        swap( graph );
        recover();
    }
    return cache_target_;
}

/**
// Save this Graph to a binary file.
*/
void Graph::save_binary()
{
    SWEET_ASSERT( build_tool_ );

    if ( !filename_.empty() )
    {
        path::Path path( filename_ );
        sweet::persist::BinaryWriter writer( build_tool_->error_policy() );
        enter( writer );
        writer.write( path.string().c_str(), "graph", *this );
        exit( writer );
    }
    else
    {
        build_tool_->error( "Unable to save a dependency graph without trying to load it first" );        
    }
}

/**
// Print the dependency graph of Targets in this Graph.
//
// @param target
//  The Target to begin printing from or null to print from the root Target of
//  this Graph.
//
// @param path
//  The path to display filenames relative to.
*/
void Graph::print_dependencies( ptr<Target> target, const std::string& directory )
{
    struct ScopedVisit
    {
        Target* target_;

        ScopedVisit( Target* target )
        : target_( target )
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( target_->is_visiting() == false );
            target_->set_visiting( true );
        }

        ~ScopedVisit()
        {
            SWEET_ASSERT( target_->is_visiting() );
            target_->set_visiting( false );
        }
    };

    struct RecursivePrinter
    {
        Graph* graph_;
        
        RecursivePrinter( Graph* graph )
        : graph_( graph )
        {
            SWEET_ASSERT( graph_ );
            graph_->begin_traversal();
        }
        
        ~RecursivePrinter()
        {
            graph_->end_traversal();
        }
    
        static const char* id( Target* target )
        {
            SWEET_ASSERT( target );
            if ( !target->get_id().empty() )
            {
                return target->get_id().c_str();
            }
            else if ( target->get_filename().empty() )
            {
                return target->get_path().c_str();
            }
            else
            {
                return target->get_filename().c_str();
            }
        }
        
        void print( Target* target, const path::Path& directory, int level )
        {
            SWEET_ASSERT( target );
            SWEET_ASSERT( level >= 0 );

            ScopedVisit visit( target );

            printf( "\n" );
            for ( int i = 0; i < level; ++i )
            {
                printf( "    " );
            }

            if ( target->get_prototype() )
            {
                printf( "%s ", target->get_prototype()->get_id().c_str() );
            }

            std::time_t timestamp = target->get_timestamp();
            struct tm* time = ::localtime( &timestamp );
            printf( "'%s' %s %04d-%02d-%02d %02d:%02d:%02d", 
                id(target),
                target->is_outdated() ? "true" : "false", 
                time ? time->tm_year + 1900 : 9999, 
                time ? time->tm_mon + 1 : 99, 
                time ? time->tm_mday : 99, 
                time ? time->tm_hour : 99, 
                time ? time->tm_min : 99, 
                time ? time->tm_sec : 99
            );


            if ( !target->get_filename().empty() )
            {
                timestamp = target->get_last_write_time();
                time = ::localtime( &timestamp );
                path::Path filename = directory.relative( path::Path(target->get_filename()) );
                printf( " '%s' %04d-%02d-%02d %02d:%02d:%02d", 
                    filename.string().c_str(),
                    time->tm_year + 1900, 
                    time->tm_mon + 1, 
                    time->tm_mday, 
                    time->tm_hour, 
                    time->tm_min, 
                    time->tm_sec 
                );
            }

            if ( !target->is_visited() )
            {
                target->set_visited( true );            
            
                const vector<Target*>& dependencies = target->get_dependencies();
                for ( vector<Target*>::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i )
                {
                    Target* dependency = *i;
                    SWEET_ASSERT( dependency );
                    if ( !dependency->is_visiting() )
                    {
                        print( dependency, directory, level + 1 );
                    }
                    else
                    {
                        BuildTool* build_tool = target->get_graph()->get_build_tool();
                        SWEET_ASSERT( build_tool );
                        build_tool->warning( "Ignoring cyclic dependency from '%s' to '%s' while printing dependencies", target->get_id().c_str(), dependency->get_id().c_str() );
                    }
                }
            }
        }
    };

    RecursivePrinter recursive_printer( this );
    recursive_printer.print( target ? target.get() : root_target_.get(), path::Path(directory), 0 );
    printf( "\n\n" );
}

/**
// Print the Target namespace of this Graph.
//
// @param target
//  The Target to begin printing from or null to print from the root Target of
//  this Graph.
*/
void Graph::print_namespace( ptr<Target> target )
{
    struct RecursivePrinter
    {
        Graph* graph_;
        
        RecursivePrinter( Graph* graph )
        : graph_( graph )
        {
            SWEET_ASSERT( graph_ );
            graph_->begin_traversal();
        }
        
        ~RecursivePrinter()
        {
            graph_->end_traversal();
        }

        static void print( ptr<Target> target, int level )
        {
            SWEET_ASSERT( target != 0 );
            SWEET_ASSERT( level >= 0 );

            if ( !target->get_id().empty() )
            {
                printf( "\n" );
                for ( int i = 0; i < level; ++i )
                {
                    printf( "    " );
                }

                if ( target->get_prototype() != 0 )
                {
                    printf( "%s ", target->get_prototype()->get_id().c_str() );
                }

                printf( "'%s'", target->get_id().c_str() );
            }

            if ( !target->is_visited() )
            {
                target->set_visited( true );            
            
                const vector<ptr<Target> >& targets = target->get_targets();
                for ( vector<ptr<Target> >::const_iterator i = targets.begin(); i != targets.end(); ++i )
                {
                    ptr<Target> target = *i;
                    SWEET_ASSERT( target );
                    print( target, level + 1 );
                }
            }
        }
    };

    RecursivePrinter recursive_printer( this );
    recursive_printer.print( target ? target : root_target_, 0 );
    printf( "\n\n" );
}

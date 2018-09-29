//
// Graph.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Graph.hpp"
#include "TargetPrototype.hpp"
#include "Target.hpp"
#include "Forge.hpp"
#include "Scheduler.hpp"
#include "System.hpp"
#include "path_functions.hpp"
#include "GraphReader.hpp"
#include "GraphWriter.hpp"
#include <assert/assert.hpp>
#include <memory>
#include <fstream>

using std::list;
using std::vector;
using std::string;
using std::unique_ptr;
using std::transform;
using namespace sweet;
using namespace sweet::forge;

/**
// Constructor.
*/
Graph::Graph()
: forge_( nullptr ),
  target_prototypes_(),
  filename_(),
  root_target_( nullptr ),
  cache_target_( nullptr ),
  traversal_in_progress_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 )
{
}

/**
// Constructor.
//
// @param forge
//  The Forge that this Graph is part of.
*/
Graph::Graph( Forge* forge )
: forge_( forge ),
  target_prototypes_(),
  filename_(),
  root_target_(),
  cache_target_(),
  traversal_in_progress_( false ),
  visited_revision_( 0 ),
  successful_revision_( 0 )
{
    SWEET_ASSERT( forge_ );
    root_target_.reset( new Target("", this) );
}

Graph::~Graph()
{
    while ( !target_prototypes_.empty() )
    {
        delete target_prototypes_.back();
        target_prototypes_.pop_back();
    }
}

/**
// Get the root Target.
//
// @return
//  The root Target.
*/
Target* Graph::root_target() const
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
Target* Graph::cache_target() const
{
    return cache_target_;
}
        
/**
// Get the Forge that this Graph is part of.
//
// @return
//  The Forge.
*/
Forge* Graph::forge() const
{
    SWEET_ASSERT( forge_ );
    return forge_;
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
bool Graph::traversal_in_progress() const
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
int Graph::visited_revision() const
{
    return visited_revision_;
}

/**
// Get the current success revision for this Graph.
//
// This is used to determine whether or not Targets have been visited 
// successfully during a pass.  See Graph::visited_revision() for a
// description of how the revision values are used.
//
// @return
//  The current successful revision for this Graph.
*/
int Graph::successful_revision() const
{
    return successful_revision_;
}

/**
// Create a new target prototype.
//
// @param id
//  The identifier of the target prototype to create.
//
// @return
//  The TargetPrototype.
*/
TargetPrototype* Graph::add_target_prototype( const std::string& id )
{   
    unique_ptr<TargetPrototype> target_prototype( new TargetPrototype(id, forge_) );
    target_prototypes_.push_back( target_prototype.get() );
    return target_prototype.release();
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
// working directory.
//
// The working directory is updated to allow Targets for C/C++ libraries to be
// lazily created as plain file Targets by depending executables before the 
// libraries themselves are defined.  When the library is lazily defined by 
// the executable reference the working directory is the working directory 
// for the executable.  When the library is defined it specifies the correct
// TargetPrototype and working directory.
//
// If \e id refers to an already existing Target that already has a 
// TargetPrototype set then an error is generated as it is not clear which
// TargetPrototype applies.
//
// Any drive portion of the path is forced to uppercase to avoid drives 
// appearing as upper- or lowercase when the build tool is run from different
// environments on Windows doesn't create separate target hierarchies.
//
// Skip over any '/' character that occurs to indicate the root 
// directory in an absolute path as this character is not expected to 
// occur as a target identifier.
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
Target* Graph::target( const std::string& id, TargetPrototype* target_prototype, Target* working_directory )
{
    boost::filesystem::path path( id );
    Target* target = working_directory && path.is_relative() ? working_directory : root_target_.get();
    SWEET_ASSERT( target );

    if ( !id.empty() )
    {
        boost::filesystem::path::const_iterator i = path.begin();

        if ( path.has_root_name() )
        {
            string element = i->generic_string();
            transform( element.begin(), element.end(), element.begin(), toupper );
            target = find_or_create_target_by_element( target, element );
            ++i;
        }

        if ( path.is_absolute() )
        {
            SWEET_ASSERT( i->generic_string() == "/" );
            ++i;
        }

        while ( i != path.end() )
        {
            target = find_or_create_target_by_element( target, i->generic_string() );
            ++i;
        }
    }
    else
    {
        unique_ptr<Target> new_target( new Target("", this) );
        Target* next_target = new_target.get();
        target->add_target( new_target.release(), target );
        target = next_target;
    }

    if ( target_prototype && target->prototype() == NULL )
    {
        target->set_prototype( target_prototype );
        target->set_working_directory( working_directory );
    }

    if ( target->working_directory() == NULL )
    {
        target->set_working_directory( working_directory );
    }

    if ( target_prototype && target->prototype() != target_prototype )
    {
        forge_->errorf( "The target '%s' has been created with prototypes '%s' and '%s'", id.c_str(), target->prototype()->id().c_str(), target_prototype ? target_prototype->id().c_str() : "none" );
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
Target* Graph::find_target( const std::string& id, Target* working_directory )
{
    Target* target = NULL;
    if ( !id.empty() )
    {
        boost::filesystem::path path( id );
        target = working_directory && path.is_relative() ? working_directory : root_target_.get();
        boost::filesystem::path::const_iterator i = path.begin();
        SWEET_ASSERT( target );

        if ( path.has_root_name() )
        {
            string element = i->generic_string();
            transform( element.begin(), element.end(), element.begin(), toupper );
            target = find_or_create_target_by_element( target, element );
            ++i;
        }

        if ( path.is_absolute() )
        {
            SWEET_ASSERT( i->generic_string() == "/" );
            ++i;
        }

        while ( i != path.end() && target )
        {
            target = find_target_by_element( target, i->generic_string() );
            ++i;
        }    
    }
    return target;
}

/**
// Find a child target of \e target with an identifier matching \e element.
//
// @param target
//  The target to find or create a child target of.
//
// @param element
//  The identifier to find or create the child target with.
//
// @return
//  The found target.
*/
Target* Graph::find_target_by_element( Target* target, const std::string& element )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( !element.empty() );

    Target* found_target = NULL;
    if ( element == "." )
    {
        found_target = target;
    }
    else if ( element == ".." )
    {
        found_target = target->parent();
    }
    else
    {
        found_target = target->find_target_by_id( element );
    }
    return found_target;
}

/**
// Find or create a child target of \e target with an identifier matching 
// \e element.
//
// @param target
//  The target to find or create a child target of.
//
// @param element
//  The identifier to find or create the child target with.
//
// @return
//  The found or created target.
*/
Target* Graph::find_or_create_target_by_element( Target* target, const std::string& element )
{
    SWEET_ASSERT( target );
    SWEET_ASSERT( !element.empty() );

    Target* found_target = NULL;
    if ( element == "." )
    {
        found_target = target;
    }
    else if ( element == ".." )
    {
        SWEET_ASSERT( target );
        SWEET_ASSERT( target->parent() );
        SWEET_ASSERT( target->parent()->graph() == this );
        found_target = target->parent();
    }
    else
    {
        found_target = target->find_target_by_id( element );
        if ( !found_target )
        {
            unique_ptr<Target> new_target( new Target(element, this) );
            found_target = new_target.get();
            target->add_target( new_target.release(), target );
            found_target->set_working_directory( target );
        }
    }
    return found_target;
}

/**
// Load a buildfile into this Graph.
//
// @param filename
//  The name of the buildfile to load.
//
// @return 
//  The number of errors that occured while loading and executing the 
//  buildfile or -1 if the buildfile yields (0 indicates successful
//  completion of the call).
*/
int Graph::buildfile( const std::string& filename )
{
    SWEET_ASSERT( forge_ );
    SWEET_ASSERT( root_target_ );     
    boost::filesystem::path path( forge_->absolute(filename) );
    SWEET_ASSERT( path.is_absolute() );   
    Target* buildfile_target = Graph::target( path.generic_string(), nullptr, nullptr );
    buildfile_target->set_filename( path.generic_string() );
    if ( cache_target_ )
    {
        cache_target_->add_explicit_dependency( buildfile_target );
    }
    return forge_->scheduler()->buildfile( path );
}

struct ScopedVisit
{
    Target* target_;

    ScopedVisit( Target* target )
    : target_( target )
    {
        SWEET_ASSERT( target_ );
        SWEET_ASSERT( target_->visiting() == false );
        target_->set_visited( true );
        target_->set_visiting( true );
    }

    ~ScopedVisit()
    {
        SWEET_ASSERT( target_ );
        SWEET_ASSERT( target_->visiting() == true );
        target_->set_visiting( false );
    }
};

struct Bind
{
    Forge* forge_;
    int failures_;
    
    Bind( Forge* forge )
    : forge_( forge ),
      failures_( 0 )
    {
        SWEET_ASSERT( forge_ );
        forge_->graph()->begin_traversal();
    }
    
    ~Bind()
    {
        forge_->graph()->end_traversal();
    }

    void visit( Target* target )
    {
        SWEET_ASSERT( target );

        if ( !target->visited() )
        {
            ScopedVisit visit( target );

            int i = 0;
            Target* dependency = target->any_dependency( i );
            while ( dependency )
            {
                if ( !dependency->visiting() )
                {
                    Bind::visit( dependency );
                }
                else
                {
                    forge_->errorf( "Cyclic dependency from %s to %s in bind", target->error_identifier().c_str(), dependency->error_identifier().c_str() );
                    dependency->set_successful( true );
                    ++failures_;
                }
                ++i;
                dependency = target->any_dependency( i );
            }

            target->bind();
            target->set_successful( true );
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
int Graph::bind( Target* target )
{
    SWEET_ASSERT( !target || target->graph() == this );

    Graph* graph = forge_->graph();
    if ( graph->traversal_in_progress() )
    {
        forge_->error( "Bind called from within another bind or postorder traversal" );
        return 0;
    }

    Bind bind( forge_ );
    bind.visit( target ? target : root_target_.get() );
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
    std::swap( root_target_, graph.root_target_ );
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
    SWEET_ASSERT( forge_ );

    struct RecursiveClear
    {
        static void clear( Target* target )
        {
            SWEET_ASSERT( target );
            target->clear_explicit_dependencies();
            target->clear_ordering_dependencies();
            target->destroy_anonymous_targets();
            target->graph()->forge()->destroy_target_lua_binding( target );

            const vector<Target*>& targets = target->targets();
            for ( vector<Target*>::const_iterator i = targets.begin(); i != targets.end(); ++i )
            {
                Target* target = *i;
                SWEET_ASSERT( target );
                RecursiveClear::clear( target );
            }
        }
    };

    RecursiveClear::clear( root_target_.get() );
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
        cache_target_ = target( filename_, NULL, NULL );
        cache_target_->set_filename( filename_ );
        bind( cache_target_ );
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
Target* Graph::load_binary( const std::string& filename )
{
    SWEET_ASSERT( !filename.empty() );
    SWEET_ASSERT( boost::filesystem::path(filename).is_absolute() );
    SWEET_ASSERT( forge_ );
    
    filename_ = filename;
    cache_target_ = NULL;

    if ( forge_->system()->exists(filename) )
    {
        std::ifstream ifstream( filename, std::ios::binary );
        GraphReader graph_reader( &ifstream, &forge_->error_policy() );
        unique_ptr<Target> root_target = graph_reader.read( filename );
        if ( root_target )
        {
            root_target_.swap( root_target );
            recover();
            return cache_target_;
        }
    }
    
    recover();
    return nullptr;
}

/**
// Save this Graph to a binary file.
*/
void Graph::save_binary()
{
    SWEET_ASSERT( forge_ );

    if ( !filename_.empty() )
    {
        std::ofstream ofstream( filename_, std::ios::binary );
        GraphWriter graph_writer( &ofstream );
        graph_writer.write( root_target_.get() );
    }
    else
    {
        forge_->error( "Unable to save a dependency graph without trying to load it first" );        
    }
}

/**
// Print the dependency graph of Targets in this Graph.
//
// @param target
//  The Target to begin printing from or null to print from the root Target of
//  this Graph.
//
// @param directory
//  The path to display filenames relative to.
*/
void Graph::print_dependencies( Target* target, const std::string& directory )
{
    struct ScopedVisit
    {
        Target* target_;

        ScopedVisit( Target* target )
        : target_( target )
        {
            SWEET_ASSERT( target_ );
            SWEET_ASSERT( target_->visiting() == false );
            target_->set_visiting( true );
        }

        ~ScopedVisit()
        {
            SWEET_ASSERT( target_->visiting() );
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
            if ( !target->id().empty() )
            {
                return target->id().c_str();
            }
            else if ( target->filenames().empty() )
            {
                return target->path().c_str();
            }
            else
            {
                return target->filename(0).c_str();
            }
        }
        
        void indent( int level )
        {
            printf( "\n" );
            for ( int i = 0; i < level; ++i )
            {
                printf( "    " );
            }
        }

        void print( Target* target, const boost::filesystem::path& directory, int level, bool ordering )
        {
            SWEET_ASSERT( target );
            SWEET_ASSERT( level >= 0 );

            indent( level );

            if ( target->prototype() )
            {
                printf( "%s ", target->prototype()->id().c_str() );
            }

            std::time_t timestamp = target->timestamp();
            struct tm* time = ::localtime( &timestamp );
            printf( "'%s' %c%c%c%c%c%c %04d-%02d-%02d %02d:%02d:%02d %s", 
                id(target),
                target->outdated() ? 'O' : 'o',
                target->changed() ? 'T' : 't',
                target->bound_to_file() ? 'F' : 'f',
                target->referenced_by_script() ? 'S' : 's',
                target->cleanable() ? 'C' : 'c',
                target->built() ? 'B' : 'b',
                time ? time->tm_year + 1900 : 9999, 
                time ? time->tm_mon + 1 : 99, 
                time ? time->tm_mday : 99, 
                time ? time->tm_hour : 99, 
                time ? time->tm_min : 99, 
                time ? time->tm_sec : 99,
                ordering ? "*" : ""
            );

            if ( !target->filenames().empty() )
            {
                timestamp = target->last_write_time();
                time = ::localtime( &timestamp );
                printf( " %04d-%02d-%02d %02d:%02d:%02d", 
                    time->tm_year + 1900, 
                    time->tm_mon + 1, 
                    time->tm_mday, 
                    time->tm_hour, 
                    time->tm_min, 
                    time->tm_sec 
                );

                const vector<string>& filenames = target->filenames();
                for ( vector<string>::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename )
                {
                    boost::filesystem::path generic_filename = sweet::forge::relative( boost::filesystem::path(*filename), directory );
                    indent( level + 1 );
                    printf( ">'%s'", generic_filename.generic_string().c_str() );
                }
            }
        }

        void print_recursively( Target* target, const boost::filesystem::path& directory, int level )
        {
            ScopedVisit visit( target );
            print( target, directory, level, false );
            if ( !target->visited() )
            {
                target->set_visited( true );            
            
                int i = 0;
                Target* dependency = target->binding_dependency( i );
                while ( dependency )
                {
                    if ( !dependency->visiting() )
                    {
                        print_recursively( dependency, directory, level + 1 );
                    }
                    else
                    {
                        Forge* forge = target->graph()->forge();
                        SWEET_ASSERT( forge );
                        forge->outputf( "Ignoring cyclic dependency from '%s' to '%s' while printing dependencies", target->id().c_str(), dependency->id().c_str() );
                    }
                    ++i;
                    dependency = target->binding_dependency( i );
                }

                i = 0;
                dependency = target->ordering_dependency( i );
                while ( dependency )
                {
                    print( dependency, directory, level + 1, true );
                    ++i;
                    dependency = target->ordering_dependency( i );
                }
            }
        }
    };

    bind( target );
    RecursivePrinter recursive_printer( this );
    recursive_printer.print_recursively( target ? target : root_target_.get(), boost::filesystem::path(directory), 0 );
    printf( "\n\n" );
}

/**
// Print the Target namespace of this Graph.
//
// @param target
//  The Target to begin printing from or null to print from the root Target of
//  this Graph.
*/
void Graph::print_namespace( Target* target )
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

        static void print( Target* target, int level )
        {
            SWEET_ASSERT( target != 0 );
            SWEET_ASSERT( level >= 0 );

            if ( !target->id().empty() )
            {
                printf( "\n" );
                for ( int i = 0; i < level; ++i )
                {
                    printf( "    " );
                }

                if ( target->prototype() != NULL )
                {
                    printf( "%s ", target->prototype()->id().c_str() );
                }

                printf( "'%s'", target->id().c_str() );
            }

            if ( !target->visited() )
            {
                target->set_visited( true );            
            
                const vector<Target*>& targets = target->targets();
                for ( vector<Target*>::const_iterator i = targets.begin(); i != targets.end(); ++i )
                {
                    Target* target = *i;
                    SWEET_ASSERT( target );
                    print( target, level + 1 );
                }
            }
        }
    };

    RecursivePrinter recursive_printer( this );
    recursive_printer.print( target ? target : root_target_.get(), 0 );
    printf( "\n\n" );
}

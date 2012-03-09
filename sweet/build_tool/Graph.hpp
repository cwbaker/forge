//
// Graph.hpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_GRAPH_HPP_INCLUDED
#define SWEET_BUILD_TOOL_GRAPH_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/pointer/ptr.hpp>
#include <sweet/error/macros.hpp>
#include <sweet/rtti/macros.hpp>

namespace sweet
{

namespace build_tool
{

class Environment;
class Rule;
class Target;
class BuildTool;

/**
// A dependency graph.
*/
class SWEET_BUILD_TOOL_DECLSPEC Graph
{
    BuildTool* build_tool_; ///< The BuildTool that this Graph is part of.
    ptr<Target> root_target_; ///< The root Target for this Graph.
    ptr<Target> cache_target_; ///< The cache Target for this Graph.
    bool loaded_from_cache_; ///< True if this Graph was loaded from a cache file otherwise false.
    bool traversal_in_progress_; ///< True when a traversal is in progress otherwise false.
    int visited_revision_; ///< The current visit revision.
    int successful_revision_; ///< The current success revision.

    public:
        Graph();
        Graph( BuildTool* build_tool );

        Target* get_root_target() const;
        Target* get_cache_target() const;
        BuildTool* get_build_tool() const;
        bool is_loaded_from_cache() const;

        void begin_traversal();
        void end_traversal();
        bool is_traversal_in_progress() const;
        int get_visited_revision() const;
        int get_successful_revision() const;             

        ptr<Target> target( const std::string& id, ptr<Rule> rule = ptr<Rule>(), ptr<Target> working_directory = ptr<Target>() );
        ptr<Target> find_target( const std::string& path, ptr<Target> working_directory );
        void insert_target( ptr<Target> target, ptr<Target> working_directory );
                
        void buildfile( const std::string& filename, ptr<Target> target );
        int bind( ptr<Target> target = ptr<Target>() );        
        void clear( const std::string& filename );
        void recover( const std::string& filename );
        Target* load_xml( const std::string& filename );
        void save_xml( const std::string& filename );
        Target* load_binary( const std::string& filename );
        void save_binary( const std::string& filename );
        void print_dependencies( ptr<Target> target );
        void print_namespace( ptr<Target> target );

        template <class Archive> void enter( Archive& archive );
        template <class Archive> void exit( Archive& archive );
        template <class Archive> void persist( Archive& archive );
};

/**
// Declare the types persisted in an Archive.
//
// @param archive
//  The Archive to declare the types with.
*/
template <class Archive> void Graph::enter( Archive& archive )
{
    using namespace sweet::persist;
    SWEET_ASSERT( build_tool_ );
    archive.set_context( SWEET_STATIC_TYPEID(BuildTool), build_tool_ );
    sweet::lua::enter( archive, build_tool_->get_script_interface()->get_lua() );
    archive.declare<Graph>( "Graph", PERSIST_NORMAL );
    archive.declare<Target>( "Target", PERSIST_NORMAL );
}

/**
// Destroy state created for persisting this Graph.
//
// @param archive
//  The Archive that state was created for.
*/
template <class Archive> void Graph::exit( Archive& archive )
{
    SWEET_ASSERT( build_tool_ );
    sweet::lua::exit( archive, build_tool_->get_script_interface()->get_lua() );
}

/**
// Persist this Graph with an Archive.
//
// @param archive
//  The Graph to persist this Archive with.
*/
template <class Archive> void Graph::persist( Archive& archive )
{
    const int BUILD_GRAPH_VERSION = 7;
    archive.enter( "Sweet Build Graph", BUILD_GRAPH_VERSION, *this );
    if ( archive.version() != BUILD_GRAPH_VERSION )
    {
        SWEET_ERROR( GraphVersionInvalidError("The graph file '%s' has an invalid version", sweet::persist::narrow(archive.get_filename()).c_str()) );
    }    
    archive.value( "root", root_target_ );
}

}

}

#endif

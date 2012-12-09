//
// Graph.hpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
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
class TargetPrototype;
class Target;
class BuildTool;

/**
// A dependency graph.
*/
class SWEET_BUILD_TOOL_DECLSPEC Graph
{
    BuildTool* build_tool_; ///< The BuildTool that this Graph is part of.
    std::string filename_; ///< The filename that this Graph was most recently loaded from.
    ptr<Target> root_target_; ///< The root Target for this Graph.
    ptr<Target> cache_target_; ///< The cache Target for this Graph.
    bool traversal_in_progress_; ///< True when a traversal is in progress otherwise false.
    int visited_revision_; ///< The current visit revision.
    int successful_revision_; ///< The current success revision.

    public:
        Graph();
        Graph( BuildTool* build_tool );

        Target* get_root_target() const;
        Target* get_cache_target() const;
        BuildTool* get_build_tool() const;

        void begin_traversal();
        void end_traversal();
        bool is_traversal_in_progress() const;
        int get_visited_revision() const;
        int get_successful_revision() const;             

        ptr<Target> target( const std::string& id, ptr<TargetPrototype> target_prototype = ptr<TargetPrototype>(), ptr<Target> working_directory = ptr<Target>() );
        ptr<Target> find_target( const std::string& path, ptr<Target> working_directory );
        void insert_target( ptr<Target> target, ptr<Target> working_directory );
                
        void buildfile( const std::string& filename, ptr<Target> target );
        int bind( ptr<Target> target = ptr<Target>() );        
        void clear();
        void recover();
        ptr<Target> load_xml( const std::string& filename );
        void save_xml();
        ptr<Target> load_binary( const std::string& filename );
        void save_binary();
        void print_dependencies( ptr<Target> target );
        void print_namespace( ptr<Target> target );

        template <class Archive> void enter( Archive& archive );
        template <class Archive> void exit( Archive& archive );
        template <class Archive> void persist( Archive& archive );
};

}

}

#endif

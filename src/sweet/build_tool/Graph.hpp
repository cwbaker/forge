#ifndef SWEET_BUILD_TOOL_GRAPH_HPP_INCLUDED
#define SWEET_BUILD_TOOL_GRAPH_HPP_INCLUDED

#include <sweet/error/macros.hpp>
#include <sweet/rtti/macros.hpp>
#include <vector>
#include <string>

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
class Graph
{
    BuildTool* build_tool_; ///< The BuildTool that this Graph is part of.
    std::vector<TargetPrototype*> target_prototypes_; ///< The TargetPrototypes that have been created.
    std::string filename_; ///< The filename that this Graph was most recently loaded from.
    Target* root_target_; ///< The root Target for this Graph.
    Target* cache_target_; ///< The cache Target for this Graph.
    bool traversal_in_progress_; ///< True when a traversal is in progress otherwise false.
    int visited_revision_; ///< The current visit revision.
    int successful_revision_; ///< The current success revision.

    public:
        Graph();
        Graph( BuildTool* build_tool );
        ~Graph();

        Target* root_target() const;
        Target* cache_target() const;
        BuildTool* build_tool() const;

        void begin_traversal();
        void end_traversal();
        bool traversal_in_progress() const;
        int visited_revision() const;
        int successful_revision() const;             

        TargetPrototype* target_prototype( const std::string& id );
        Target* target( const std::string& id, TargetPrototype* target_prototype = NULL, Target* working_directory = NULL );
        Target* find_target( const std::string& path, Target* working_directory );
                
        void buildfile( const std::string& filename );
        int bind( Target* target = NULL );        
        void swap( Graph& graph );
        void clear();
        void recover();
        Target* load_xml( const std::string& filename );
        void save_xml();
        Target* load_binary( const std::string& filename );
        void save_binary();
        void print_dependencies( Target* target, const std::string& directory );
        void print_namespace( Target* target );

        template <class Archive> void enter( Archive& archive );
        template <class Archive> void exit( Archive& archive );
        template <class Archive> void persist( Archive& archive );
};

}

}

#endif

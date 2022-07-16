#ifndef FORGE_GRAPH_HPP_INCLUDED
#define FORGE_GRAPH_HPP_INCLUDED

#include <error/macros.hpp>
#include <vector>
#include <string>
#include <memory>

namespace sweet
{

namespace forge
{

class Environment;
class Rule;
class Toolset;
class Target;
class Forge;

/**
// A dependency graph.
*/
class Graph
{
    Forge* forge_; ///< The Forge that this Graph is part of.
    std::vector<Rule*> rules_; ///< The Rules that have been created.
    std::vector<Toolset*> toolsets_; ///< The Toolsets that have been created.
    std::string filename_; ///< The filename that this Graph was most recently loaded from.
    std::unique_ptr<Target> root_target_; ///< The root Target for this Graph.
    Target* cache_target_; ///< The cache Target for this Graph.
    bool traversal_in_progress_; ///< True when a traversal is in progress otherwise false.
    int visited_revision_; ///< The current visit revision.
    int successful_revision_; ///< The current success revision.

    public:
        Graph();
        Graph( Forge* forge );
        ~Graph();

        const std::vector<Toolset*> toolsets() const;
        Target* root_target() const;
        Target* cache_target() const;
        Forge* forge() const;

        void begin_traversal();
        void end_traversal();
        bool traversal_in_progress() const;
        int visited_revision() const;
        int successful_revision() const;             

        Rule* add_rule( const std::string& id );
        Toolset* add_toolset( const std::string& id );
        Target* target( const std::string& id );
        Target* add_or_find_target( const std::string& id, Target* working_directory = nullptr );
        Target* find_target( const std::string& path, Target* working_directory );
        Target* find_target_by_element( Target* target, const std::string& element );
        Target* find_or_create_target_by_element( Target* target, const std::string& element );
                
        int buildfile( const std::string& filename );
        int bind( Target* target = NULL );        
        void swap( Graph& graph );
        void clear();
        void recover();
        Target* load_binary( const std::string& filename );
        void save_binary();
        void print_dependencies( Target* target, const std::string& directory );
        void print_namespace( Target* target );
};

}

}

#endif

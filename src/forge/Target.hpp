#ifndef FORGE_TARGET_HPP_INCLUDED
#define FORGE_TARGET_HPP_INCLUDED

#include <ctime>
#include <string>
#include <vector>
#include <stdint.h>

namespace sweet
{

namespace forge
{

class GraphWriter;
class GraphReader;
class TargetPrototype;
class Graph;
class Forge;

/**
// A Target.
*/
class Target
{
    std::string id_; ///< The identifier of this Target.
    mutable std::string path_; ///< The full path to this Target in the Target namespace.
    mutable std::string branch_; ///< The branch path to this Target in the Target namespace.
    Graph* graph_; ///< The Graph that this Target is part of.
    TargetPrototype* prototype_; ///< The TargetPrototype for this Target or null if this Target has no TargetPrototype.
    std::time_t timestamp_; ///< The timestamp for this Target.
    std::time_t last_write_time_; ///< The last write time of the file that this Target is bound to.
    uint64_t hash_; ///< The hash for this Target the last time that it was built.
    uint64_t pending_hash_; ///< The hash for this Target when it was created in the current run.
    bool outdated_; ///< Whether or not this Target is out of date.
    bool changed_; ///< Whether or not this Target's timestamp has changed since the last time it was bound to a file.
    bool bound_to_file_; ///< Whether or not this Target is bound to a file.
    bool bound_to_dependencies_; ///< Whether or not this Target is bound to its dependencies.
    bool referenced_by_script_; ///< Whether or not this Target is referenced by a scripting object.  
    bool cleanable_; ///< Whether or not this Target is able to be cleaned.
    bool built_; ///< Whether or not this Target has had `Target::clear_implicit_dependencies()` called on it.
    Target* working_directory_; ///< The Target that relative paths expressed when this Target is visited are relative to.
    Target* parent_; ///< The parent of this Target in the Target namespace or null if this Target has no parent.
    std::vector<Target*> targets_; ///< The children of this Target in the Target namespace.
    std::vector<Target*> dependencies_; ///< Explicit dependencies.
    std::vector<Target*> implicit_dependencies_; ///< Implicit dependencies.
    std::vector<Target*> ordering_dependencies_; ///< Targets that must build before this Target is built.
    std::vector<Target*> passive_dependencies_; ///< Passive dependencies of this Target.
    std::vector<std::string> filenames_; ///< The filenames of this Target.
    bool visiting_; ///< Whether or not this Target is in the process of being visited.
    int visited_revision_; ///< The visited revision the last time this Target was visited.
    int successful_revision_; ///< The successful revision the last time this Target was successfully visited.
    int postorder_height_; ///< The height of this Target in the current or most recent dependency graph traversal.
    int anonymous_; ///< The anonymous index for this Target that will generate the next anonymous identifier requested from this Target.

    public:
        Target();
        Target( const std::string& id, Graph* graph );
        ~Target();
        void recover( Graph* graph );

        const std::string& id() const;
        const std::string& path() const;
        const std::string& branch() const;
        Graph* graph() const;
        bool anonymous() const;
        uint64_t hash() const;

        void set_prototype( TargetPrototype* target_prototype );
        TargetPrototype* prototype() const;

        void bind();
        void bind_to_file();
        void bind_to_dependencies();
        void bind_to_hash();
        void set_hash( uint64_t hash );

        void set_referenced_by_script( bool referenced_by_script );
        bool referenced_by_script() const;

        void set_cleanable( bool cleanable );
        bool cleanable() const;

        void set_built( bool built );
        bool built() const;

        void set_timestamp( std::time_t timestamp );
        std::time_t timestamp() const;
        std::time_t last_write_time() const;

        void set_outdated( bool outdated );
        bool outdated() const;
        bool changed() const;
        bool bound_to_file() const;

        void add_filename( const std::string& filename );
        void set_filename( const std::string& filename, int index );
        void clear_filenames( int start, int finish );
        const std::string& filename( int index ) const;
        const std::vector<std::string>& filenames() const;
        std::string directory( int index ) const;

        void set_working_directory( Target* target );
        Target* working_directory() const;

        void set_parent( Target* target );
        Target* parent() const;

        void add_target( Target* target, Target* this_target );
        void destroy_anonymous_targets();
        Target* find_target_by_id( const std::string& id ) const;
        const std::vector<Target*>& targets() const;

        void add_explicit_dependency( Target* target );
        void clear_explicit_dependencies();
        void add_implicit_dependency( Target* target );
        void remove_implicit_dependency( Target* target );
        void clear_implicit_dependencies();
        void add_ordering_dependency( Target* target );
        void clear_ordering_dependencies();
        void add_passive_dependency( Target* target );
        void clear_passive_dependencies();
        void remove_dependency( Target* target );
        bool is_explicit_dependency( Target* target ) const;
        bool is_implicit_dependency( Target* target ) const;
        bool is_ordering_dependency( Target* target ) const;
        bool is_passive_dependency( Target* target ) const;
        bool is_dependency( Target* target ) const;
        Target* explicit_dependency( int n ) const;
        Target* implicit_dependency( int n ) const;
        Target* ordering_dependency( int n ) const;
        Target* passive_dependency( int n ) const;
        Target* any_dependency( int n ) const;

        bool buildable() const;
        std::string error_identifier() const;
        std::string failed_dependencies() const;

        void set_visiting( bool visiting );
        bool visiting() const;

        void set_visited( bool visited );
        bool visited() const;

        void set_successful( bool successful );
        bool successful() const;

        void set_postorder_height( int height );
        int postorder_height() const;
        
        int next_anonymous_index();

        void write( GraphWriter& writer );
        void read( GraphReader& reader );
        void resolve( const GraphReader& reader );
        template <class Archive> void persist( Archive& archive );
};

}

}

#endif

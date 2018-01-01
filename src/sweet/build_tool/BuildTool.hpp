#ifndef SWEET_BUILD_TOOL_BUILDTOOL_HPP_INCLUDED
#define SWEET_BUILD_TOOL_BUILDTOOL_HPP_INCLUDED

#include <boost/filesystem/path.hpp>
#include <string>
#include <vector>

struct lua_State;

namespace sweet
{

namespace error
{

class ErrorPolicy;

}

namespace build_tool
{

class Context;
class BuildToolEventSink;
class Reader;
class Executor;
class Scheduler;
class System;
class TargetPrototype;
class Target;
class Graph;
class LuaBuildTool;

/**
// BuildTool library main class.
*/
class BuildTool
{
    error::ErrorPolicy& error_policy_;
    BuildToolEventSink* event_sink_; ///< The EventSink for this BuildTool or null if this BuildTool has no EventSink.
    LuaBuildTool* lua_build_tool_; ///< The Lua bindings to the build tool library.
    System* system_; ///< The System that provides access to the operating system.
    Reader* reader_; ///< The reader that filters executable output and dependencies.
    Graph* graph_; ///< The dependency graph of targets used to determine which targets are outdated.
    Scheduler* scheduler_; ///< The scheduler that schedules environments to process jobs in the dependency graph.
    Executor* executor_; ///< The executor that schedules threads to process commands.
    boost::filesystem::path root_directory_; ///< The full path to the root directory.
    boost::filesystem::path initial_directory_; ///< The full path to the initial directory.
    boost::filesystem::path home_directory_; ///< The full path to the user's home directory.
    boost::filesystem::path executable_directory_; ///< The full path to the build executable directory.
    bool stack_trace_enabled_; ///< Print stack traces on error when true.

    public:
        BuildTool( const std::string& initial_directory, error::ErrorPolicy& error_policy, BuildToolEventSink* event_sink );
        ~BuildTool();

        error::ErrorPolicy& error_policy() const;
        System* system() const;
        Reader* reader() const;
        Graph* graph() const;
        Scheduler* scheduler() const;
        Executor* executor() const;
        Context* context() const;
        lua_State* lua_state() const;

        const boost::filesystem::path& root() const;
        boost::filesystem::path root( const boost::filesystem::path& path ) const;
        const boost::filesystem::path& initial() const;
        boost::filesystem::path initial( const boost::filesystem::path& path ) const;
        const boost::filesystem::path& home() const;
        boost::filesystem::path home( const boost::filesystem::path& path ) const;
        const boost::filesystem::path& executable() const;
        boost::filesystem::path executable( const boost::filesystem::path& path ) const;
        boost::filesystem::path absolute( const boost::filesystem::path& path ) const;
        boost::filesystem::path relative( const boost::filesystem::path& path ) const;

        void set_stack_trace_enabled( bool stack_trace_enabled );
        bool stack_trace_enabled() const;
        void set_maximum_parallel_jobs( int maximum_parallel_jobs );
        int maximum_parallel_jobs() const;
        void set_build_hooks_library( const std::string& build_hooks_library );
        const std::string& build_hooks_library() const;

        void search_up_for_root_directory( const std::string& directory );
        void assign_global_variables( const std::vector<std::string>& assignments_and_commands );
        void execute( const std::string& command );

        void create_target_lua_binding( Target* target );
        void recover_target_lua_binding( Target* target );
        void update_target_lua_binding( Target* target );
        void destroy_target_lua_binding( Target* target );
        void create_target_prototype_lua_binding( TargetPrototype* target_prototype );
        void destroy_target_prototype_lua_binding( TargetPrototype* target_prototype );

        void output( const char* format, ... );
        void error( const char* format, ... );
};

}

}

#endif

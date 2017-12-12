#ifndef SWEET_BUILD_TOOL_BUILDTOOL_HPP_INCLUDED
#define SWEET_BUILD_TOOL_BUILDTOOL_HPP_INCLUDED

#include <sweet/fs/BasicPath.hpp>
#include <string>
#include <vector>

namespace sweet
{

namespace error
{

class ErrorPolicy;

}

namespace lua
{

class Lua;

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
    int warning_level_; ///< The warning level to report warnings at.
    lua::Lua* lua_; ///< The Lua virtual machine.
    LuaBuildTool* lua_build_tool_; ///< The Lua bindings to the build tool library.
    System* system_; ///< The System that provides access to the operating system.
    Reader* reader_; ///< The reader that filters executable output and dependencies.
    Executor* executor_; ///< The executor that schedules threads to process commands.
    Scheduler* scheduler_; ///< The scheduler that schedules environments to process jobs in the dependency graph.
    Graph* graph_; ///< The dependency graph of targets used to determine which targets are outdated.
    fs::Path root_directory_; ///< The full path to the root directory.
    fs::Path initial_directory_; ///< The full path to the initial directory.
    fs::Path home_directory_; ///< The full path to the user's home directory.
    fs::Path executable_directory_; ///< The full path to the build executable directory.

    public:
        BuildTool( const std::string& initial_directory, error::ErrorPolicy& error_policy, BuildToolEventSink* event_sink );
        ~BuildTool();

        error::ErrorPolicy& error_policy() const;
        System* system() const;
        Graph* graph() const;
        Reader* reader() const;
        Executor* executor() const;
        Scheduler* scheduler() const;
        Context* context() const;
        lua::Lua* lua() const;
        LuaBuildTool* lua_build_tool() const;

        const fs::Path& root() const;
        fs::Path root( const fs::Path& path ) const;
        const fs::Path& initial() const;
        fs::Path initial( const fs::Path& path ) const;
        const fs::Path& home() const;
        fs::Path home( const fs::Path& path ) const;
        const fs::Path& executable() const;
        fs::Path executable( const fs::Path& path ) const;
        fs::Path absolute( const fs::Path& path ) const;

        void set_warning_level( int warning_level );
        int warning_level() const;
        void set_stack_trace_enabled( bool stack_trace_enabled );
        bool stack_trace_enabled() const;
        void set_maximum_parallel_jobs( int maximum_parallel_jobs );
        int maximum_parallel_jobs() const;
        void set_build_hooks_library( const std::string& build_hooks_library );
        const std::string& build_hooks_library() const;

        void search_up_for_root_directory( const std::string& directory );
        void assign( const std::vector<std::string>& assignments_and_commands );
        void execute( const std::string& filename, const std::vector<std::string>& assignments_and_commands );
        void execute( const char* start, const char* finish );

        void create_target_lua_binding( Target* target );
        void recover_target_lua_binding( Target* target );
        void update_target_lua_binding( Target* target, TargetPrototype* target_prototype );
        void destroy_target_lua_binding( Target* target );
        void create_target_prototype_lua_binding( TargetPrototype* target_prototype );
        void destroy_target_prototype_lua_binding( TargetPrototype* target_prototype );

        void output( const char* text );
        void warning( const char* format, ... );
        void error( const char* format, ... );
};

}

}

#endif

#ifndef SWEET_BUILD_TOOL_BUILDTOOL_HPP_INCLUDED
#define SWEET_BUILD_TOOL_BUILDTOOL_HPP_INCLUDED

#include "declspec.hpp"
#include <string>
#include <vector>

namespace sweet
{

namespace error
{

class ErrorPolicy;

}

namespace build_tool
{

class BuildToolEventSink;
class ScriptInterface;
class Reader;
class Executor;
class Scheduler;
class OsInterface;
class TargetPrototype;
class Graph;

/**
// BuildTool library main class.
*/
class SWEET_BUILD_TOOL_DECLSPEC BuildTool
{
    error::ErrorPolicy& error_policy_;
    BuildToolEventSink* event_sink_; ///< The EventSink for this BuildTool or null if this BuildTool has no EventSink.
    int warning_level_; ///< The warning level to report warnings at.
    OsInterface* os_interface_; ///< The OsInterface that provides access to the operating system.
    ScriptInterface* script_interface_; ///< The ScriptInterface that provides the API used by Lua scripts.
    Reader* reader_; ///< The reader that filters executable output and dependencies.
    Executor* executor_; ///< The executor that schedules threads to process commands.
    Scheduler* scheduler_; ///< The scheduler that schedules environments to process jobs in the dependency graph.
    Graph* graph_; ///< The dependency graph of targets used to determine which targets are outdated.

    public:
        BuildTool( const std::string& initial_directory, error::ErrorPolicy& error_policy, BuildToolEventSink* event_sink );
        ~BuildTool();

        error::ErrorPolicy& error_policy() const;
        OsInterface* os_interface() const;
        ScriptInterface* script_interface() const;
        Graph* graph() const;
        Reader* reader() const;
        Executor* executor() const;
        Scheduler* scheduler() const;

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
        void output( const char* text );
        void warning( const char* format, ... );
        void error( const char* format, ... );
};

}

}

#endif

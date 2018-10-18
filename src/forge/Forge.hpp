#ifndef FORGE_FORGE_HPP_INCLUDED
#define FORGE_FORGE_HPP_INCLUDED

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

namespace forge
{

class Context;
class ForgeEventSink;
class Reader;
class Executor;
class Scheduler;
class System;
class TargetPrototype;
class Target;
class Graph;
class LuaForge;

/**
// Forge library main class.
*/
class Forge
{
    error::ErrorPolicy& error_policy_;
    ForgeEventSink* event_sink_; ///< The EventSink for this Forge or null if this Forge has no EventSink.
    LuaForge* lua_forge_; ///< The Lua bindings to the Forge library.
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
        Forge( const std::string& initial_directory, error::ErrorPolicy& error_policy, ForgeEventSink* event_sink );
        ~Forge();

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
        void set_forge_hooks_library( const std::string& forge_hooks_library );
        const std::string& forge_hooks_library() const;

        void set_root_directory( const std::string& root_directory );
        void assign_global_variables( const std::vector<std::string>& assignments_and_commands );
        void execute( const std::string& filename, const std::string& command );
        void script( const std::string& script );

        void create_target_lua_binding( Target* target );
        void update_target_lua_binding( Target* target );
        void destroy_target_lua_binding( Target* target );
        void create_target_prototype_lua_binding( TargetPrototype* target_prototype );
        void destroy_target_prototype_lua_binding( TargetPrototype* target_prototype );

        void outputf( const char* format, ... );
        void errorf( const char* format, ... );
        void output( const char* message );
        void error( const char* message );
};

}

}

#endif

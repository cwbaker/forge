#ifndef FORGE_CONTEXT_HPP_INCLUDED
#define FORGE_CONTEXT_HPP_INCLUDED

#include <boost/filesystem/path.hpp>
#include <vector>

struct lua_State;

namespace sweet
{

namespace forge
{

class Job;
class Target;
class Forge;

/**
// Provides context for a script to interact with its outside environment.
*/
class Context 
{
    Forge* forge_; ///< The Forge that this context is part of.
    lua_State* lua_state_; ///< The Lua coroutine that this Context uses to execute scripts.
    int lua_state_reference_; ///< The Lua reference to the Lua coroutine above (see `luaL_ref()`).
    Target* current_buildfile_; ///< The current buildfile target for this context.
    Target* working_directory_; ///< The current working directory for this context.
    std::vector<boost::filesystem::path> directories_; ///< The stack of working directories for this context (the element at the top is the current working directory).
    Job* job_; ///< The current Job for this context.
    int exit_code_; ///< The exit code from the Job that was most recently executed by this context.
    Context* buildfile_calling_context_; ///< The Context that made a `buildfile()` call and yielded.
    bool prune_; ///< Set true if any Lua script calls `prune()` on a traversal.

    public:
        Context( Forge* forge );
        ~Context();

        lua_State* lua_state() const;
        const boost::filesystem::path& directory() const;
        Target* current_buildfile() const;
        Target* working_directory() const;
        Job* job() const;
        int exit_code() const;
        Context* buildfile_calling_context() const;
        bool prune() const;
        boost::filesystem::path absolute( const boost::filesystem::path& path ) const;
        boost::filesystem::path relative( const boost::filesystem::path& path ) const;

        void reset_directory_to_target( Target* directory );
        void reset_directory( const boost::filesystem::path& directory );
        void change_directory( const boost::filesystem::path& directory );
        void push_directory( const boost::filesystem::path& directory );
        void pop_directory();
        void set_current_buildfile( Target* buildfile );
        void set_job( Job* job );
        void set_exit_code( int exit_code );
        void set_buildfile_calling_context( Context* context );
        void set_prune( bool prune );
};

}

}

#endif

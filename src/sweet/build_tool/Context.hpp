#ifndef SWEET_BUILD_TOOL_CONTEXT_HPP_INCLUDED
#define SWEET_BUILD_TOOL_CONTEXT_HPP_INCLUDED

#include <sweet/lua/LuaThread.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>

namespace sweet
{

namespace build_tool
{

class Job;
class Target;
class BuildTool;

/**
// Provides context for a script to interact with its outside environment.
*/
class Context 
{
    BuildTool* build_tool_; ///< The BuildTool that this context is part of.
    lua::LuaThread context_thread_; ///< The LuaThread that this context uses to execute scripts in.
    Target* working_directory_; ///< The current working directory for this context.
    std::vector<boost::filesystem::path> directories_; ///< The stack of working directories for this context (the element at the top is the current working directory).
    Job* job_; ///< The current Job for this context.
    int exit_code_; ///< The exit code from the Command that was most recently executed by this context.

    public:
        Context( const boost::filesystem::path& directory, BuildTool* build_tool );

        lua::LuaThread& context_thread();

        void reset_directory_to_target( Target* directory );
        void reset_directory( const boost::filesystem::path& directory );
        void change_directory( const boost::filesystem::path& directory );
        void push_directory( const boost::filesystem::path& directory );
        void pop_directory();
        const boost::filesystem::path& directory() const;
        Target* working_directory() const;

        void set_job( Job* job );
        Job* job() const;

        void set_exit_code( int exit_code );
        int exit_code() const;

        boost::filesystem::path absolute( const boost::filesystem::path& path ) const;
        boost::filesystem::path relative( const boost::filesystem::path& path ) const;
        static boost::filesystem::path absolute( const boost::filesystem::path& path, const boost::filesystem::path& base_path );
        static boost::filesystem::path relative( const boost::filesystem::path& path, const boost::filesystem::path& base_path );
};

}

}

#endif

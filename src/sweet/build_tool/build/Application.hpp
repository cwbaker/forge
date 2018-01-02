#ifndef APPLICATION_HPP_INCLUDED
#define APPLICATION_HPP_INCLUDED

#include <sweet/build_tool/BuildToolEventSink.hpp>

namespace sweet
{

namespace build_tool
{

class BuildTool;

class Application : public BuildToolEventSink
{
    int result_;

    public:
        Application( int argc, char** argv );
        int get_result() const;

    private:
        void build_tool_output( BuildTool* build_tool, const char* message );
        void build_tool_warning( BuildTool* build_tool, const char* message );
        void build_tool_error( BuildTool* build_tool, const char* message );
};

}

}

#endif

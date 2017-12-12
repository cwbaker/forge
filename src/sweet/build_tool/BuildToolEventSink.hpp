#ifndef SWEET_BUILD_TOOL_BUILDTOOLEVENTSINK_HPP_INCLUDED
#define SWEET_BUILD_TOOL_BUILDTOOLEVENTSINK_HPP_INCLUDED

#include "declspec.hpp"

namespace sweet
{

namespace build_tool
{

class BuildTool;

/**
// An interface for classes that wish to receive events from a 
// BuildTool.
*/
class SWEET_BUILD_TOOL_DECLSPEC BuildToolEventSink
{
    public:
        virtual ~BuildToolEventSink();
        virtual void build_tool_output( BuildTool* build_tool, const char* message );
        virtual void build_tool_warning( BuildTool* build_tool, const char* message );
        virtual void build_tool_error( BuildTool* build_tool, const char* message );
};

}

}

#endif

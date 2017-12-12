
#ifndef ERROR_CHECKER_HPP_INCLUDED
#define ERROR_CHECKER_HPP_INCLUDED

#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <string>

namespace sweet
{

namespace build_tool
{

class BuildTool;

struct ErrorChecker : public BuildToolEventSink
{
    std::string message;
    int errors;
    
    ErrorChecker();
    virtual ~ErrorChecker();
    void build_tool_error( BuildTool* build_tool, const char* mmessage );
    void test( const char* script );
};    

}

}

#endif


#ifndef ERROR_CHECKER_HPP_INCLUDED
#define ERROR_CHECKER_HPP_INCLUDED

#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <sweet/error/ErrorPolicy.hpp>
#include <string>
#include <vector>

namespace sweet
{

namespace build_tool
{

class BuildTool;

struct ErrorChecker : public error::ErrorPolicy, public BuildToolEventSink
{
    std::vector<std::string> messages;
    int errors;
    
    ErrorChecker();
    virtual ~ErrorChecker();
    void build_tool_error( BuildTool* build_tool, const char* mmessage );
    void test( const char* script );

private:
    void report_error( const char* message );
};    

}

}

#endif

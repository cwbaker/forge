
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/error/ErrorPolicy.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace sweet::build_tool;

ErrorChecker::ErrorChecker()
: build_tool::BuildToolEventSink(),
  error::ErrorPolicyEventSink(),
  messages(),
  errors( 0 )
{
}

ErrorChecker::~ErrorChecker()
{
}

void ErrorChecker::build_tool_error( BuildTool* build_tool, const char* mmessage )
{
    messages.push_back( mmessage );
    ++errors;
}    

void ErrorChecker::error( const char* mmessage )
{
    messages.push_back( mmessage );
    ++errors;
}    

void ErrorChecker::test( const char* script )
{
    SWEET_ASSERT( script );
    messages.clear();
    errors = 0;          
    error::ErrorPolicy error_policy( this );  
    BuildTool build_tool( boost::filesystem::initial_path<boost::filesystem::path>().string(), error_policy, this );
    build_tool.execute( script, script + strlen(script) );
}

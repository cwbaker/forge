
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace sweet::build_tool;

ErrorChecker::ErrorChecker()
: message(),
  errors( 0 )
{
}

ErrorChecker::~ErrorChecker()
{
}

void ErrorChecker::build_tool_error( BuildTool* build_tool, const char* mmessage )
{
    message = mmessage;
    ++errors;
}    

void ErrorChecker::test( const char* script )
{
    SWEET_ASSERT( script );
    message.clear();
    errors = 0;            
    BuildTool build_tool( boost::filesystem::initial_path<boost::filesystem::path>().string(), this );
    build_tool.execute( script, script + strlen(script) );
}

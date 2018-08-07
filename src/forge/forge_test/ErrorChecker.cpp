
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/error/ErrorPolicy.hpp>
#include <sweet/assert/assert.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using std::string;
using namespace boost::filesystem;
using namespace sweet::build_tool;

ErrorChecker::ErrorChecker()
: error::ErrorPolicy(),
  build_tool::BuildToolEventSink(),
  messages(),
  errors( 0 )
{
}

ErrorChecker::~ErrorChecker()
{
}

void ErrorChecker::build_tool_error( BuildTool* /*build_tool*/, const char* message )
{
    messages.push_back( message );
    ++errors;
}    

void ErrorChecker::test( const char* script )
{
    SWEET_ASSERT( script );
    messages.clear();
    errors = 0;          
    path path = initial_path<boost::filesystem::path>();
    BuildTool build_tool( path.string(), *this, this );
    build_tool.set_root_directory( path.generic_string() );
    build_tool.script( string(script) );
}

void ErrorChecker::report_error( const char* message )
{
    // ::fputs( message, stdout );
    messages.push_back( message );
    ++errors;
}    

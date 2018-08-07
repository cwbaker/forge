
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <forge/Forge.hpp>
#include <error/ErrorPolicy.hpp>
#include <assert/assert.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using std::string;
using namespace boost::filesystem;
using namespace sweet::forge;

ErrorChecker::ErrorChecker()
: error::ErrorPolicy(),
  forge::ForgeEventSink(),
  messages(),
  errors( 0 )
{
}

ErrorChecker::~ErrorChecker()
{
}

void ErrorChecker::forge_error( Forge* /*forge*/, const char* message )
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
    Forge forge( path.string(), *this, this );
    forge.set_root_directory( path.generic_string() );
    forge.script( string(script) );
}

void ErrorChecker::report_error( const char* message )
{
    // ::fputs( message, stdout );
    messages.push_back( message );
    ++errors;
}    

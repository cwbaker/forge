
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <forge/Forge.hpp>
#include <error/ErrorPolicy.hpp>
#include <assert/assert.hpp>
#include <filesystem>

using std::string;
using namespace std::filesystem;
using namespace sweet::forge;

ErrorChecker::ErrorChecker()
: error::ErrorPolicy(),
  messages(),
  errors( 0 )
{
}

ErrorChecker::~ErrorChecker()
{
}

void ErrorChecker::test( const char* script )
{
    SWEET_ASSERT( script );
    messages.clear();
    errors = 0;
    path path = current_path();
    Forge forge( path.string(), *this );
    forge.set_root_directory( path.generic_string() );
    forge.script( string(script) );
}

void ErrorChecker::report_error( const char* message )
{
    // ::fputs( message, stdout );
    messages.push_back( message );
    ++errors;
}    

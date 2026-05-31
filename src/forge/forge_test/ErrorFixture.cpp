
#include "stdafx.hpp"
#include "ErrorFixture.hpp"
#include <assert/assert.hpp>
#include <cstdio>
#include <filesystem>

using std::string;
using namespace std::filesystem;
using namespace sweet::forge;

ErrorFixture::ErrorFixture()
: error::ErrorPolicy()
, messages_()
, show_errors_( false )
{
}

ErrorFixture::~ErrorFixture()
{
}

const char* ErrorFixture::error_message( int index ) const
{
    return index >= 0 && index < int(messages_.size()) ? messages_[index].c_str() : "";
}

void ErrorFixture::quiet()
{
    show_errors_ = false;
}

void ErrorFixture::verbose()
{
    show_errors_ = true;
}

void ErrorFixture::clear()
{
    messages_.clear();
}

void ErrorFixture::report_error( const char* message )
{
    SWEET_ASSERT( message );
    messages_.push_back( message );
    if ( show_errors_ )
    {
        fputs( message, stderr );
        fputs( "\n", stderr );
        fflush( stderr );
    }
}

void ErrorFixture::report_print( const char* message )
{
    SWEET_ASSERT( message );
    if ( show_errors_ )
    {
        fputs( message, stdout );
        fputs( "\n", stdout );
        fflush( stdout );
    }
}

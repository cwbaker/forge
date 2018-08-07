
#include "stdafx.hpp"
#include "FileChecker.hpp"
#include <assert/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>

using std::vector;
using namespace sweet::forge;

FileChecker::FileChecker()
: ErrorChecker(),
  files_()
{
}

FileChecker::~FileChecker()
{
    for ( vector<const char*>::const_iterator i = files_.begin(); i != files_.end(); ++i )
    {
        const char* filename = *i;
        boost::filesystem::remove( filename );
    }
    files_.clear();
}        

void FileChecker::create( const char* filename, const char* content, std::time_t last_write_time )
{
    SWEET_ASSERT( filename );
    SWEET_ASSERT( content );
    
    boost::filesystem::remove( filename );
    std::ofstream file( filename );
    file.write( content, strlen(content) );
    file.close();
    files_.push_back( filename );

    if ( last_write_time != 0 )
    {
        touch( filename, last_write_time );
    }
}

void FileChecker::remove( const char* filename )
{
    SWEET_ASSERT( filename );
    boost::filesystem::remove( filename );    
}

void FileChecker::touch( const char* filename, std::time_t last_write_time )
{
    SWEET_ASSERT( filename );            
    boost::filesystem::last_write_time( filename, last_write_time );
}

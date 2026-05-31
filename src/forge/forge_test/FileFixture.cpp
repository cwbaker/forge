
#include "stdafx.hpp"
#include "FileFixture.hpp"
#include <assert/assert.hpp>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>

using std::string;
using std::vector;
using namespace sweet::forge;

FileFixture::FileFixture()
: files_()
{
}

FileFixture::~FileFixture()
{
    for ( vector<string>::const_iterator i = files_.begin(); i != files_.end(); ++i )
    {
        const char* filename = i->c_str();
        std::filesystem::remove( filename );
    }
    files_.clear();
}

void FileFixture::create( const char* filename, const char* content, std::time_t last_write_time )
{
    SWEET_ASSERT( filename );
    SWEET_ASSERT( content );

    std::filesystem::remove( filename );
    std::ofstream file( filename );
    file.write( content, strlen(content) );
    file.close();
    files_.push_back( string(filename) );

    if ( last_write_time != 0 )
    {
        touch( filename, last_write_time );
    }
}

void FileFixture::remove( const char* filename )
{
    SWEET_ASSERT( filename );
    std::filesystem::remove( filename );
}

void FileFixture::touch( const char* filename, std::time_t last_write_time )
{
    SWEET_ASSERT( filename );
    const auto sctp = std::chrono::system_clock::from_time_t( last_write_time );
    const auto ftime = std::chrono::time_point_cast<std::filesystem::file_time_type::duration>(
        sctp - std::chrono::system_clock::now() + std::filesystem::file_time_type::clock::now()
    );
    std::filesystem::last_write_time( filename, ftime );
}

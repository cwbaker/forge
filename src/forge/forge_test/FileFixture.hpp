#ifndef FILE_FIXTURE_HPP_INCLUDED
#define FILE_FIXTURE_HPP_INCLUDED

#include <string>
#include <vector>
#include <ctime>

namespace sweet
{

namespace forge
{

class FileFixture
{
    std::vector<std::string> files_;

public:
    FileFixture();
    ~FileFixture();
    void create( const char* filename, const char* content, std::time_t last_write_time = 0 );
    void remove( const char* filename );
    void touch( const char* filename, std::time_t last_write_time );
};

}

}

#endif

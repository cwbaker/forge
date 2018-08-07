
#ifndef FILE_CHECKER_HPP_INCLUDED
#define FILE_CHECKER_HPP_INCLUDED

#include "ErrorChecker.hpp"
#include <vector>
#include <ctime>

namespace sweet
{

namespace forge
{

struct FileChecker : public ErrorChecker
{
    std::vector<const char*> files_;
    
    FileChecker();
    ~FileChecker();
    void create( const char* filename, const char* content, std::time_t last_write_time = 0 );
    void remove( const char* filename );
    void touch( const char* filename, std::time_t last_write_time );
};

}

}

#endif

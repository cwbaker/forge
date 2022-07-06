#ifndef FILE_CHECKER_HPP_INCLUDED
#define FILE_CHECKER_HPP_INCLUDED

#include "ErrorChecker.hpp"
#include <string>
#include <vector>
#include <ctime>

namespace sweet
{

namespace forge
{

class FileChecker : public ErrorChecker
{
    std::vector<std::string> files_;
    
public:
    FileChecker();
    ~FileChecker();
    void create( const char* filename, const char* content, std::time_t last_write_time = 0 );
    void remove( const char* filename );
    void touch( const char* filename, std::time_t last_write_time );
};

}

}

#endif

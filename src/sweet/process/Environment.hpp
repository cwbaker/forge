#ifndef SWEET_PROCESS_ENVIRONMENT_HPP_INCLUDED
#define SWEET_PROCESS_ENVIRONMENT_HPP_INCLUDED

#include <vector>

namespace sweet
{
    
namespace process
{

/**
// An array of key value pairs to store the environment passed to spawn a new
// process.
*/
class Environment
{
    std::vector<char*> values_;
    std::vector<char> buffer_;

public:
    Environment( unsigned int values_reserve = 8, unsigned int buffer_reserve = 1024 );
    char* const* values() const;
    const char* buffer() const;
    void append( const char* key, const char* value );
    void finish();
};
    
}

}

#endif

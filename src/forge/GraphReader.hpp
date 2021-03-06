#ifndef FORGE_GRAPHREADER_HPP_INCLUDED
#define FORGE_GRAPHREADER_HPP_INCLUDED

#include <map>
#include <vector>
#include <string>
#include <istream>
#include <memory>
#include <ctime>
#include <stdint.h>

namespace sweet
{

namespace error
{

class ErrorPolicy;

}

namespace forge
{

class Target;

class GraphReader
{
    std::istream* istream_;
    error::ErrorPolicy* error_policy_;
    std::map<const void*, void*> address_by_old_address_;

public:
    GraphReader( std::istream* ostream, error::ErrorPolicy* error_policy );
    void* find_address_by_old_address( const void* old_address ) const;
    std::unique_ptr<Target> read( const std::string& filename );
    void object_address( void* address );
    void value( bool* value );
    void value( int* value );
    void value( uint64_t* value );
    void value( std::time_t* value );
    void value( std::string* value );
    void value( char* value, size_t size );
    void value( std::vector<std::string>* values );
    void value( std::vector<Target*>* values );
    void refer( std::vector<Target*>* references );
};

}

}

#endif

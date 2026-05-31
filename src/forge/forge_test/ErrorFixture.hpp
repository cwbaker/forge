
#ifndef ERROR_FIXTURE_HPP_INCLUDED
#define ERROR_FIXTURE_HPP_INCLUDED

#include <error/ErrorPolicy.hpp>
#include <string>
#include <vector>

namespace sweet
{

namespace forge
{

class Forge;

class ErrorFixture : public error::ErrorPolicy
{
    std::vector<std::string> messages_;
    bool show_errors_;

public:
    ErrorFixture();
    virtual ~ErrorFixture();
    const char* error_message( int index ) const;
    void quiet();
    void verbose();
    void clear();

private:
    void report_error( const char* message ) override;
    void report_print( const char* message ) override;
};

}

}

#endif


#ifndef ERROR_CHECKER_HPP_INCLUDED
#define ERROR_CHECKER_HPP_INCLUDED

#include <forge/ForgeEventSink.hpp>
#include <error/ErrorPolicy.hpp>
#include <string>
#include <vector>

namespace sweet
{

namespace forge
{

class Forge;

struct ErrorChecker : public error::ErrorPolicy, public ForgeEventSink
{
    std::vector<std::string> messages;
    int errors;
    
    ErrorChecker();
    virtual ~ErrorChecker();
    void forge_error( Forge* forge, const char* mmessage );
    void test( const char* script );

private:
    void report_error( const char* message );
};    

}

}

#endif

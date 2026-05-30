#pragma once

#include <error/ErrorPolicy.hpp>

namespace sweet
{

namespace forge
{

class ForgeErrorPolicy : public error::ErrorPolicy
{
private:
    void report_error( const char* message ) override;
    void report_print( const char* message ) override;
};

}

}

#ifndef SWEET_ERROR_ERRORPOLICYEVENTSINK_HPP_INCLUDED
#define SWEET_ERROR_ERRORPOLICYEVENTSINK_HPP_INCLUDED

#include "declspec.hpp"

namespace sweet
{

namespace error
{

class ErrorPolicyEventSink
{
public:
    virtual ~ErrorPolicyEventSink();
    virtual void error( const char* message );
};

}

}

#endif

//
// ErrorPolicyEventSink.hpp
// Copyright (c) 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_ERROR_ERRORPOLICYEVENTSINK_HPP_INCLUDED
#define SWEET_ERROR_ERRORPOLICYEVENTSINK_HPP_INCLUDED

#include "declspec.hpp"

namespace sweet
{

namespace error
{

class SWEET_ERROR_DECLSPEC ErrorPolicyEventSink
{
public:
    virtual ~ErrorPolicyEventSink();
    virtual void error( const char* message );
};

}

}

#endif

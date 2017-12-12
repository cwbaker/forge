//
// ErrorPolicy.hpp
// Copyright (c) 2001 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_ERROR_ERRORPOLICY_HPP_INCLUDED
#define SWEET_ERROR_ERRORPOLICY_HPP_INCLUDED

#include "declspec.hpp"

namespace std
{

class exception;

}

namespace sweet
{

namespace error
{

class ErrorPolicyEventSink;

/**
// An interface for providing runtime error handling policies.
*/
class SWEET_ERROR_DECLSPEC ErrorPolicy
{
    static const int ERROR_STACK_SIZE = 8;
    ErrorPolicyEventSink* event_sink_;
    int stack_index_;
    int errors_ [ERROR_STACK_SIZE];

public:
    ErrorPolicy( ErrorPolicyEventSink* event_sink = 0 );
    void push_errors();
    int pop_errors();
    int errors() const;
    void error( bool condition, const char* format, ... );
};

}

}

#endif

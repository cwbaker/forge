//
// ErrorPolicyEventSink.cpp
// Copyright (c) 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ErrorPolicyEventSink.hpp"
#include <assert/assert.hpp>

using namespace sweet::error;

ErrorPolicyEventSink::~ErrorPolicyEventSink()
{
}

void ErrorPolicyEventSink::error( const char* message )
{
    SWEET_ASSERT( message );
}

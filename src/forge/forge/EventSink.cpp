//
// EventSink.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include "EventSink.hpp"
#include <stdio.h>
#include <assert/assert.hpp>

using namespace sweet::forge;

void EventSink::forge_output( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    fputs( message, stdout );
    fputs( "\n", stdout );
    fflush( stdout );
}

void EventSink::forge_warning( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    fputs( "forge: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}

void EventSink::forge_error( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    fputs( "forge: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}

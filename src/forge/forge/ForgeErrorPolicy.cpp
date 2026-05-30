//
// ForgeErrorPolicy.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include "ForgeErrorPolicy.hpp"
#include <stdio.h>
#include <assert/assert.hpp>

using namespace sweet::forge;

void ForgeErrorPolicy::report_print( const char* message )
{
    SWEET_ASSERT( message );
    fputs( message, stdout );
    fputs( "\n", stdout );
    fflush( stdout );
}

void ForgeErrorPolicy::report_error( const char* message )
{
    SWEET_ASSERT( message );
    fputs( "forge: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}

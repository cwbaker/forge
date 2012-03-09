//
// main.cpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <exception>
#include <stdio.h>
#include <stdlib.h>

using namespace sweet::build_tool;

int main( int argc, char** argv )
{
    int result = EXIT_FAILURE;

    try
    {
        Application application( argc, argv );
        result = application.get_result();
    }

    catch ( const std::exception& exception )
    {
        fprintf( stderr, "build: %s.\n", exception.what() );
        result = EXIT_FAILURE;
    }

    catch ( ... )
    {
        fprintf( stderr, "build: An unexpected error occured.\n" );
        result = EXIT_FAILURE;
    }

    return result;
}

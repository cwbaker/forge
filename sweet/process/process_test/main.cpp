//
// main.cpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <exception>

using namespace sweet;
using namespace sweet::process;

int main( int argc, char** argv )
{
    try
    {
        Application application( argc, argv );
        return application.result();
    }

    catch( const std::exception& exception )
    {
        fprintf( stderr, "test: %s.\n", exception.what() );
        return EXIT_FAILURE;
    }    
}

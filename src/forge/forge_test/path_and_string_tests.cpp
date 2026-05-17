//
// path_and_string_tests.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include <forge/Forge.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( path_and_string_tests )
{
    TEST_FIXTURE( ForgeLuaFixture, path_and_string_tests )
    {
        int errors = forge->file( "path_and_string_tests.lua" );
        CHECK( errors == 0 );
    }
}

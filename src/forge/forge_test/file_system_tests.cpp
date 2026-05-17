//
// file_system_tests.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include <forge/Forge.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( file_system_tests )
{
    TEST_FIXTURE( ForgeLuaFixture, file_system_tests )
    {
        int errors = forge->file( "file_system_tests.lua" );
        CHECK( errors == 0 );
    }
}

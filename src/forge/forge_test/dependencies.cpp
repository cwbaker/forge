//
// dependencies.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include <forge/Forge.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( dependencies )
{
    TEST_FIXTURE( ForgeLuaFixture, transitive_dependencies )
    {
        int errors = forge->file( "transitive_dependencies.lua" );
        CHECK( errors == 0 );
    }
}

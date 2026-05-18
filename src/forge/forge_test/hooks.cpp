//
// hooks.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include <luaxx/luaxx_unit/LuaUnitTest.hpp>
#include <forge/Forge.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( hooks )
{
    TEST_FIXTURE( ForgeLuaFixture, hooks )
    {
        int errors = forge->file( "hooks_tests.lua" );
        CHECK( errors == 0 );
    }
}

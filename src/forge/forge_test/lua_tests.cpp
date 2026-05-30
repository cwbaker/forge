//
// lua_tests.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include <forge/Forge.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( lua_tests )
{
    TEST_FIXTURE( ForgeLuaFixture, path_tests )
    {
        int errors = forge->file( "path_tests.lua" );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( ForgeLuaFixture, string_tests )
    {
        int errors = forge->file( "string_tests.lua" );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( ForgeLuaFixture, file_system_tests )
    {
        int errors = forge->file( "file_system_tests.lua" );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( ForgeLuaFixture, hooks )
    {
        int errors = forge->file( "hooks_tests.lua" );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( ForgeLuaFixture, preorder )
    {
        int errors = forge->file( "preorder.lua" );
        CHECK_EQUAL( 8, errors );
    }

    TEST_FIXTURE( ForgeLuaFixture, transitive_dependencies )
    {
        int errors = forge->file( "transitive_dependencies.lua" );
        CHECK( errors == 0 );
    }
}

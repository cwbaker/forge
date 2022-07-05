//
// preorder.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeLuaFixture.hpp"
#include <forge/Forge.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( preorder )
{
    TEST_FIXTURE( ForgeLuaFixture, preorder )
    {
        forge->file( "preorder.lua" );
    }
}

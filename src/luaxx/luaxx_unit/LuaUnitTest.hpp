#ifndef LUAXX_LUAUNITTEST_HPP_INCLUDED
#define LUAXX_LUAUNITTEST_HPP_INCLUDED

#include <string>

struct lua_State;

namespace sweet
{

namespace error
{

class ErrorPolicy;

}

}

namespace UnitTest
{

class TestDetails;
class Timer;

}

namespace luaxx
{

class Lua;

/**
// Provide Lua bindings to Noel Llopis's UnitTest++.
//
// Tests are written in a style that mirrors UnitTest++ in C++:
//
//     SUITE 'string-tests';
//     TEST_FIXTURE( _unit_test_setup, 'lower_converts_to_lowercase', function()
//         CHECK_EQUAL( 'foo', lower('FOO') );
//     end );
//
// `SUITE()`, `TEST()`, and `TEST_FIXTURE()` are implemented in Lua
// (defined by `create()`) rather than in C so that tests are able to yield the
// coroutine that they run on -- e.g. while waiting for a spawned process to
// finish.  A C implementation would call each test through `lua_pcall()`,
// putting a non-yieldable C-call boundary between the test and the coroutine
// and causing any yield to fail with "attempt to yield across a C-call
// boundary".  The Lua driver instead invokes each test through Lua's own
// (yieldable) `xpcall()` and calls back into the thin C functions below only
// to bracket each test with the UnitTest++ bookkeeping.
*/
class LuaUnitTest
{
    lua_State* lua_state_;
    sweet::error::ErrorPolicy* error_policy_;

    // Bookkeeping for the test currently being run by the Lua `TEST()` /
    // `TEST_FIXTURE()` driver.  These fields persist between the
    // `start_test()` and `finish_test()` calls that bracket each
    // test.  `details_` holds pointers into `running_suite_name_`,
    // `running_test_name_`, and `running_test_file_`, so those strings must
    // outlive it.
    std::string running_suite_name_;
    std::string running_test_name_;
    std::string running_test_file_;
    const UnitTest::TestDetails* previous_details_;
    UnitTest::TestDetails* details_;
    UnitTest::Timer* timer_;

public:
    LuaUnitTest();
    LuaUnitTest( lua_State* lua_state, sweet::error::ErrorPolicy* error_policy );
    ~LuaUnitTest();
    void create( lua_State* lua_state, sweet::error::ErrorPolicy* error_policy );
    void run_tests( const char* filename );

private:
    static int start_test( lua_State* lua_state );
    static int finish_test( lua_State* lua_state );
    static int fail_test( lua_State* lua_state );
    static int check( lua_State* lua_state );
    static int check_equal( lua_State* lua_state );
    static int check_close( lua_State* lua_state );
    static int line( lua_State* lua_state );
};

}

#endif

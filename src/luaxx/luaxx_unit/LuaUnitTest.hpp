#ifndef LUAXX_LUAUNITTEST_HPP_INCLUDED
#define LUAXX_LUAUNITTEST_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace error
{

class ErrorPolicy;

}

}

namespace luaxx
{

class Lua;

/**
// Provide Lua bindings to Noel Llopis's UnitTest++.
*/
class LuaUnitTest
{
    lua_State* lua_state_;
    sweet::error::ErrorPolicy* error_policy_;

public:
    LuaUnitTest();
    LuaUnitTest( lua_State* lua_state, sweet::error::ErrorPolicy* error_policy );
    void create( lua_State* lua_state, sweet::error::ErrorPolicy* error_policy );
    void run_tests( const char* filename );
    
private:
    static int test_suite( lua_State* lua_state );
    static int check( lua_State* lua_state );
    static int check_equal( lua_State* lua_state );
    static int check_close( lua_State* lua_state );
    static int line( lua_State* lua_state );
};

}

#endif

#pragma once

struct lua_State;

namespace luaxx
{

class LuaUnitTest;

}

namespace sweet
{

namespace forge
{

class FileFixture;
class ErrorFixture;
class Forge;

class ForgeLuaFixture
{
public:
    forge::Forge* forge;
    luaxx::LuaUnitTest* lua_unit_test;
    FileFixture* file_fixture;
    ErrorFixture* error_fixture;

    ForgeLuaFixture();
    ~ForgeLuaFixture();

private:
    static int test_suite( lua_State* lua_state );
    static int quiet( lua_State* lua_state );
    static int verbose( lua_State* lua_state );
    static int error_message( lua_State* lua_state );
    static int create( lua_State* lua_state );
    static int remove( lua_State* lua_state );
    static int touch( lua_State* lua_state );
};

}

}

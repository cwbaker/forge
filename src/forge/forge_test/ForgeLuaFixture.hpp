#pragma once

#include <forge/ForgeEventSink.hpp>
#include <error/ErrorPolicy.hpp>

struct lua_State;

namespace luaxx
{

class LuaUnitTest;

}

namespace sweet
{

namespace forge
{

class FileChecker;
class Forge;

class ForgeLuaFixture : public ForgeEventSink, public error::ErrorPolicy
{
public:
    forge::Forge* forge;
    luaxx::LuaUnitTest* lua_unit_test;
    FileChecker* file_checker;
    bool show_errors;

    ForgeLuaFixture();
    ~ForgeLuaFixture();

private:
    void report_error( const char* message ) override;
    void report_print( const char* message ) override;
    void forge_output( Forge* /*forge*/, const char* message ) override;
    void forge_warning( Forge* /*forge*/, const char* message ) override;
    void forge_error( Forge* /*forge*/, const char* message ) override;
    static int quiet( lua_State* lua_state );
    static int verbose( lua_State* lua_state );
    static int create( lua_State* lua_state );
    static int remove( lua_State* lua_state );
    static int touch( lua_State* lua_state );
};

}

}

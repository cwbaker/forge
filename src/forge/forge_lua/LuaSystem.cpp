//
// LuaSystem.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaSystem.hpp"
#include "LuaForge.hpp"
#include "types.hpp"
#include <forge/Forge.hpp>
#include <forge/System.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>

using std::string;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

LuaSystem::LuaSystem()
{
}

LuaSystem::~LuaSystem()
{
    destroy();
}

void LuaSystem::create( Forge* forge, lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, -1) );

    destroy();

    static const luaL_Reg functions[] = 
    {
        { "getenv", &LuaSystem::getenv },
        { "sleep", &LuaSystem::sleep },
        { "ticks", &LuaSystem::ticks },
        { "operating_system", &LuaSystem::operating_system },
        { NULL, NULL }
    };
    lua_pushlightuserdata( lua_state, forge );
    luaL_setfuncs( lua_state, functions, 1 );
}

void LuaSystem::destroy()
{
}

int LuaSystem::getenv( lua_State* lua_state )
{
    const int FORGE = 1;
    const int KEY = 2;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    const char* key = luaL_checkstring( lua_state, KEY ); 
    const char* value = forge->system()->getenv( key );
    if ( value )
    {
        lua_pushstring( lua_state, value );
        return 1;
    }
    return 0;
}

int LuaSystem::sleep( lua_State* lua_state )
{
    const int FORGE = 1;
    const int MILLISECONDS = 2;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    float milliseconds = static_cast<float>( luaL_checknumber(lua_state, MILLISECONDS) );
    forge->system()->sleep( milliseconds );
    return 0;
}

int LuaSystem::ticks( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    float ticks = forge->system()->ticks();
    lua_pushnumber( lua_state, ticks );
    return 1;
}

int LuaSystem::operating_system( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    const char* operating_system = forge->system()->operating_system();
    lua_pushstring( lua_state, operating_system );
    return 1;
}

//
// LuaSystem.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaSystem.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/System.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using std::string;
using namespace sweet;
using namespace sweet::build_tool;

LuaSystem::LuaSystem()
{
}

LuaSystem::~LuaSystem()
{
    destroy();
}

void LuaSystem::create( BuildTool* build_tool, lua_State* lua_state )
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
    lua_pushlightuserdata( lua_state, build_tool );
    luaL_setfuncs( lua_state, functions, 1 );
}

void LuaSystem::destroy()
{
}

int LuaSystem::getenv( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    const int KEY = 1;
    const char* key = luaL_checkstring( lua_state, KEY );
    const char* value = build_tool->system()->getenv( key );
    if ( value )
    {
        lua_pushstring( lua_state, value );
        return 1;
    }
    return 0;
}

int LuaSystem::sleep( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    const int MILLISECONDS = 1;
    float milliseconds = luaL_checknumber( lua_state, MILLISECONDS );
    build_tool->system()->sleep( milliseconds );
    return 0;
}

int LuaSystem::ticks( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    float ticks = build_tool->system()->ticks();
    lua_pushnumber( lua_state, ticks );
    return 1;
}

int LuaSystem::operating_system( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    const char* operating_system = build_tool->system()->operating_system();
    lua_pushstring( lua_state, operating_system );
    return 1;
}

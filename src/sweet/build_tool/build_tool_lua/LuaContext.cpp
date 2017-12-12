//
// LuaContext.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaContext.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/Context.hpp>
#include <sweet/fs/BasicPath.ipp>
#include <stdlib.h>

using std::string;
using namespace sweet;
using namespace sweet::build_tool;

LuaContext::LuaContext()
{
}

LuaContext::~LuaContext()
{
    destroy();
}

void LuaContext::create( BuildTool* build_tool, lua_State* lua_state )
{
    SWEET_ASSERT( build_tool );
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, -1) );

    const luaL_Reg working_directory_based_functions[] = 
    {
        { "cd", &LuaContext::cd },
        { "pushd", &LuaContext::pushd },
        { "popd", &LuaContext::popd },
        { "pwd", &LuaContext::pwd },
        { "absolute", &LuaContext::absolute },
        { "relative", &LuaContext::relative },
        { "root", &LuaContext::root },
        { "initial", &LuaContext::initial },
        { "executable", &LuaContext::executable },
        { "home", &LuaContext::home },
        { NULL, NULL }
    };
    lua_pushlightuserdata( lua_state, build_tool );
    luaL_setfuncs( lua_state, working_directory_based_functions, 1 );

    static const luaL_Reg functions[] = 
    {
        { "lower", &LuaContext::lower },
        { "upper", &LuaContext::upper },
        { "native", &LuaContext::native },
        { "branch", &LuaContext::branch },
        { "leaf", &LuaContext::leaf },
        { "basename", &LuaContext::basename },
        { "extension", &LuaContext::extension },
        { "is_absolute", &LuaContext::is_absolute },
        { "is_relative", &LuaContext::is_relative },
        { NULL, NULL }
    };
    luaL_setfuncs( lua_state, functions, 0 );
}

void LuaContext::destroy()
{
}

int LuaContext::cd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    build_tool->context()->change_directory( fs::Path(string(path, length)) );
    return 0;
}

int LuaContext::pushd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    build_tool->context()->push_directory( fs::Path(string(path, length)) );
    return 0;
}

int LuaContext::popd( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    build_tool->context()->pop_directory();
    return 0;
}

int LuaContext::pwd( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    const fs::Path& path = build_tool->context()->directory();
    lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    return 1;
}

int LuaContext::absolute( lua_State* lua_state )
{
    const int PATH = 1;
    const int BASE_PATH = 2;
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_checkstring( lua_state, BASE_PATH );
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), fs::Path(base_path) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( build_tool );
        fs::Path path = build_tool->absolute( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaContext::relative( lua_State* lua_state )
{
    const int PATH = 1;
    const int BASE_PATH = 2;
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_checkstring( lua_state, BASE_PATH );
        fs::Path path = fs::relative( fs::Path(luaL_checkstring(lua_state, PATH)), fs::Path(base_path) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( build_tool );
        const fs::Path& base_path = build_tool->context()->directory();
        fs::Path path = fs::relative( fs::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaContext::root( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = build_tool->root();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = build_tool->root( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaContext::initial( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = build_tool->initial();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = build_tool->initial( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaContext::executable( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = build_tool->executable();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = build_tool->executable( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaContext::home( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = build_tool->home();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = build_tool->home( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaContext::lower( lua_State* lua_state )
{
    const int STRING = 1;
    const char* value = luaL_checkstring( lua_state, STRING );
    string lowercase_value;
    lowercase_value.reserve( strlen(value) );
    while ( *value ) 
    {
        lowercase_value.push_back( tolower(*value) );
        ++value;
    }
    lua_pushlstring( lua_state, lowercase_value.c_str(), lowercase_value.length() );
    return 1;
}

int LuaContext::upper( lua_State* lua_state )
{
    const int STRING = 1;
    const char* value = luaL_checkstring( lua_state, STRING );
    string uppercase_value;
    uppercase_value.reserve( strlen(value) );
    while ( *value ) 
    {
        uppercase_value.push_back( toupper(*value) );
        ++value;
    }
    lua_pushlstring( lua_state, uppercase_value.c_str(), uppercase_value.length() );
    return 1;
}

int LuaContext::native( lua_State* lua_state )
{
    const int PATH = 1;
    string native_string = fs::Path( luaL_checkstring(lua_state, PATH) ).native_string();
    lua_pushlstring( lua_state, native_string.c_str(), native_string.length() );
    return 1;
}

int LuaContext::branch( lua_State* lua_state )
{
    const int PATH = 1;
    fs::Path path = fs::Path( luaL_checkstring(lua_state, PATH) ).branch();
    lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    return 1;
}

int LuaContext::leaf( lua_State* lua_state )
{
    const int PATH = 1;
    string leaf = fs::Path( luaL_checkstring(lua_state, PATH) ).leaf();
    lua_pushlstring( lua_state, leaf.c_str(), leaf.length() );
    return 1;
}

int LuaContext::basename( lua_State* lua_state )
{
    const int PATH = 1;
    string basename = fs::Path( luaL_checkstring(lua_state, PATH) ).basename();
    lua_pushlstring( lua_state, basename.c_str(), basename.length() );
    return 1;
}

int LuaContext::extension( lua_State* lua_state )
{
    const int PATH = 1;
    string extension = fs::Path( luaL_checkstring(lua_state, PATH) ).extension();
    lua_pushlstring( lua_state, extension.c_str(), extension.length() );
    return 1;
}

int LuaContext::is_absolute( lua_State* lua_state )
{
    const int PATH = 1;
    bool absolute = fs::Path( luaL_checkstring(lua_state, PATH) ).is_absolute();
    lua_pushboolean( lua_state, absolute ? 1 : 0 );
    return 1;
}

int LuaContext::is_relative( lua_State* lua_state )
{
    const int PATH = 1;
    bool relative = fs::Path( luaL_checkstring(lua_state, PATH) ).is_relative();
    lua_pushboolean( lua_state, relative ? 1 : 0 );
    return 1;
}

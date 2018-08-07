//
// LuaContext.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaContext.hpp"
#include "LuaForge.hpp"
#include "types.hpp"
#include <forge/path_functions.hpp>
#include <forge/Forge.hpp>
#include <forge/Context.hpp>
#include <luaxx/luaxx.hpp>
#include <boost/filesystem/path.hpp>
#include <stdlib.h>

using std::string;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

LuaContext::LuaContext()
{
}

LuaContext::~LuaContext()
{
    destroy();
}

void LuaContext::create( Forge* forge, lua_State* lua_state )
{
    SWEET_ASSERT( forge );
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
    lua_pushlightuserdata( lua_state, forge );
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
    const int FORGE = 1;
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length );
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    forge->context()->change_directory( boost::filesystem::path(string(path, length)) );
    return 0;
}

int LuaContext::pushd( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length );
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    forge->context()->push_directory( boost::filesystem::path(string(path, length)) );
    return 0;
}

int LuaContext::popd( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    forge->context()->pop_directory();
    return 0;
}

int LuaContext::pwd( lua_State* lua_state )
{
    const int FORGE = 1;
    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    const boost::filesystem::path& path = forge->context()->directory();
    lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    return 1;
}

int LuaContext::absolute( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;
    const int BASE_PATH = 3;

    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_tolstring( lua_state, BASE_PATH, nullptr );
        const char* relative_path = !lua_isnoneornil( lua_state, PATH ) ? luaL_tolstring( lua_state, PATH, nullptr ) : "";
        boost::filesystem::path path = sweet::forge::absolute( boost::filesystem::path(relative_path), boost::filesystem::path(base_path) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    else
    {
        const char* relative_path = !lua_isnoneornil( lua_state, PATH ) ? luaL_tolstring( lua_state, PATH, nullptr ) : "";
        boost::filesystem::path path = forge->absolute( boost::filesystem::path(relative_path) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    return 1;
}

int LuaContext::relative( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;
    const int BASE_PATH = 3;

    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    if ( !lua_isnoneornil(lua_state, BASE_PATH) )
    {
        const char* base_path = luaL_tolstring( lua_state, BASE_PATH, nullptr );
        const char* absolute_path = !lua_isnoneornil( lua_state, PATH ) ? luaL_tolstring( lua_state, PATH, nullptr ) : "";
        boost::filesystem::path path = sweet::forge::relative( boost::filesystem::path(absolute_path), boost::filesystem::path(base_path) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    else
    {
        const char* absolute_path = !lua_isnoneornil( lua_state, PATH ) ? luaL_tolstring( lua_state, PATH, nullptr ) : "";
        boost::filesystem::path path = forge->relative( boost::filesystem::path(absolute_path) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    return 1;
}

int LuaContext::root( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;

    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const boost::filesystem::path& path = forge->root();
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    else
    {
        boost::filesystem::path path = forge->root( boost::filesystem::path(luaL_tolstring(lua_state, PATH, nullptr)) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }

    return 1;
}

int LuaContext::initial( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;

    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const boost::filesystem::path& path = forge->initial();
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    else
    {
        boost::filesystem::path path = forge->initial( boost::filesystem::path(luaL_tolstring(lua_state, PATH, nullptr)) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }

    return 1;
}

int LuaContext::executable( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;

    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const boost::filesystem::path& path = forge->executable();
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    else
    {
        boost::filesystem::path path = forge->executable( boost::filesystem::path(luaL_tolstring(lua_state, PATH, nullptr)) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }

    return 1;
}

int LuaContext::home( lua_State* lua_state )
{
    const int FORGE = 1;
    const int PATH = 2;

    Forge* forge = (Forge*) luaxx_check( lua_state, FORGE, FORGE_TYPE );

    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const boost::filesystem::path& path = forge->home();
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }
    else
    {
        boost::filesystem::path path = forge->home( boost::filesystem::path(luaL_tolstring(lua_state, PATH, nullptr)) );
        lua_pushlstring( lua_state, path.generic_string().c_str(), path.generic_string().length() );
    }

    return 1;
}

int LuaContext::lower( lua_State* lua_state )
{
    const int STRING = 2;
    size_t length = 0;
    const char* value = luaL_tolstring( lua_state, STRING, &length );
    string lowercase_value;
    lowercase_value.reserve( length );
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
    const int STRING = 2;
    size_t length = 0;
    const char* value = luaL_tolstring( lua_state, STRING, &length );
    string uppercase_value;
    uppercase_value.reserve( length );
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
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length );
    string native_path = boost::filesystem::path( string(path, length) ).make_preferred().string();
    lua_pushlstring( lua_state, native_path.c_str(), native_path.length() );
    return 1;
}

int LuaContext::branch( lua_State* lua_state )
{
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length ); 
    string branch = boost::filesystem::path( string(path, length) ).parent_path().generic_string();
    lua_pushlstring( lua_state, branch.c_str(), branch.length() );
    return 1;
}

int LuaContext::leaf( lua_State* lua_state )
{
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length );
    string leaf = boost::filesystem::path( string(path, length) ).filename().generic_string();
    lua_pushlstring( lua_state, leaf.c_str(), leaf.length() );
    return 1;
}

int LuaContext::basename( lua_State* lua_state )
{
    const int PATH = 2;
    string basename = boost::filesystem::path( luaL_tolstring(lua_state, PATH, nullptr) ).stem().generic_string();
    lua_pushlstring( lua_state, basename.c_str(), basename.length() );
    return 1;
}

int LuaContext::extension( lua_State* lua_state )
{
    const int PATH = 2;
    string extension = boost::filesystem::path( luaL_tolstring(lua_state, PATH, nullptr) ).extension().generic_string();
    lua_pushlstring( lua_state, extension.c_str(), extension.length() );
    return 1;
}

int LuaContext::is_absolute( lua_State* lua_state )
{
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length );
    bool absolute = boost::filesystem::path( string(path, length) ).is_absolute();
    lua_pushboolean( lua_state, absolute ? 1 : 0 );
    return 1;
}

int LuaContext::is_relative( lua_State* lua_state )
{
    const int PATH = 2;
    size_t length = 0;
    const char* path = luaL_tolstring( lua_state, PATH, &length );
    bool relative = boost::filesystem::path( string(path, length) ).is_relative();
    lua_pushboolean( lua_state, relative ? 1 : 0 );
    return 1;
}

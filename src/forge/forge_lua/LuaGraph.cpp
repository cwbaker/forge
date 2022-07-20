//
// LuaGraph.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaGraph.hpp"
#include "types.hpp"
#include <forge/Context.hpp>
#include <forge/Forge.hpp>
#include <forge/Scheduler.hpp>
#include <forge/Graph.hpp>
#include <forge/Toolset.hpp>
#include <forge/Target.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <algorithm>

using std::min;
using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

LuaGraph::LuaGraph()
{
}

LuaGraph::~LuaGraph()
{
    destroy();
}

void LuaGraph::create( Forge* forge, lua_State* lua_state )
{
    SWEET_ASSERT( forge );
    SWEET_ASSERT( lua_state );

    destroy();

    static const luaL_Reg functions[] = 
    {
        { "add_toolset", &LuaGraph::add_toolset },
        { "all_toolsets", &LuaGraph::all_toolsets },
        { "find_target", &LuaGraph::find_target },
        { "anonymous", &LuaGraph::anonymous },
        { "current_buildfile", &LuaGraph::current_buildfile },
        { "working_directory", &LuaGraph::working_directory },
        { "buildfile", &LuaGraph::buildfile },
        { "preorder", &LuaGraph::preorder },
        { "postorder", &LuaGraph::postorder },
        { "print_dependencies", &LuaGraph::print_dependencies },
        { "print_namespace", &LuaGraph::print_namespace },
        { "prune", &LuaGraph::prune },
        { "wait", &LuaGraph::wait },
        { "clear", &LuaGraph::clear },
        { "load_binary", &LuaGraph::load_binary },
        { "save_binary", &LuaGraph::save_binary },
        { NULL, NULL }
    };
    lua_pushglobaltable( lua_state );
    lua_pushlightuserdata( lua_state, forge );
    luaL_setfuncs( lua_state, functions, 1 );
    lua_pop( lua_state, 1 );
}

void LuaGraph::destroy()
{
}

int LuaGraph::add_toolset( lua_State* lua_state )
{
    try
    {
        const int FORGE = lua_upvalueindex( 1 );
        const int IDENTIFIER = 1;
        Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
        string id = luaL_optstring( lua_state, IDENTIFIER, "" );
        Toolset* toolset = forge->graph()->add_toolset( id );
        forge->create_toolset_lua_binding( toolset );
        luaxx_push( lua_state, toolset );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int LuaGraph::all_toolsets_iterator( lua_State* lua_state )
{
    const int GRAPH = 1;
    const int INDEX = 2;

    Graph* graph = (Graph*) lua_touserdata( lua_state, GRAPH );
    SWEET_ASSERT( graph );
    const vector<Toolset*>& toolsets = graph->toolsets();
    int index = int(lua_tointeger(lua_state, INDEX));
    SWEET_ASSERT( index >= 0 );

    if ( index < int(toolsets.size()) )
    {
        lua_pushinteger( lua_state, index + 1 );

        Toolset* toolset = toolsets[index];
        SWEET_ASSERT( toolset );
        luaxx_push( lua_state, toolset );

        const string& id = toolset->id();
        lua_pushlstring( lua_state, id.c_str(), id.length() );
        return 3;
    }
    return 0;
}

int LuaGraph::all_toolsets( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Graph* graph = forge->graph();
    lua_pushcfunction( lua_state, &LuaGraph::all_toolsets_iterator );
    lua_pushlightuserdata( lua_state, graph );
    lua_pushinteger( lua_state, 0 );
    return 3;
}

int LuaGraph::find_target( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int IDENTIFIER = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    const char* id = luaL_checkstring( lua_state, IDENTIFIER );
    Target* target = forge->graph()->find_target( string(id), context->working_directory() );
    if ( target && !target->referenced_by_script() )
    {
        forge->create_target_lua_binding( target );
    }
    luaxx_push( lua_state, target );
    return 1;
}

int LuaGraph::anonymous( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    Target* working_directory = context->working_directory();
    char anonymous [256];
    size_t length = sprintf( anonymous, "$$%d", working_directory->next_anonymous_index() );
    anonymous[min(length, sizeof(anonymous) - 1)] = 0;
    lua_pushlstring( lua_state, anonymous, length );
    return 1;
}

int LuaGraph::current_buildfile( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    Target* target = context->current_buildfile();
    if ( target && !target->referenced_by_script() )
    {
        forge->create_target_lua_binding( target );
    }
    luaxx_push( lua_state, target );
    return 1;
}

int LuaGraph::working_directory( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    Target* target = context->working_directory();
    if ( target && !target->referenced_by_script() )
    {
        forge->create_target_lua_binding( target );
    }
    luaxx_push( lua_state, target );
    return 1;
}

int LuaGraph::buildfile( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    const char* filename = luaL_checkstring( lua_state, FILENAME );
    int errors = forge->graph()->buildfile( string(filename) );
    if ( errors >= 0 )
    {
        lua_pushinteger( lua_state, errors );
        return 1;
    }
    return lua_yield( lua_state, 0 );
}

int LuaGraph::preorder( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TARGET = 1;
    const int FUNCTION = 2;

    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Graph* graph = forge->graph();
    if ( graph->traversal_in_progress() )
    {
        return luaL_error( lua_state, "Preorder called from within preorder or postorder" );
    }
     
    Target* target = nullptr;
    if ( !lua_isnoneornil(lua_state, TARGET) )
    {
        target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    }

    lua_pushvalue( lua_state, FUNCTION );
    int function = luaL_ref( lua_state, LUA_REGISTRYINDEX );
    int failures = forge->scheduler()->preorder( target, function );
    lua_pushinteger( lua_state, failures );
    luaL_unref( lua_state, LUA_REGISTRYINDEX, function );
    return 1;
}

int LuaGraph::postorder( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TARGET = 1;
    const int FUNCTION = 2;

    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Graph* graph = forge->graph();
    if ( graph->traversal_in_progress() )
    {
        return luaL_error( lua_state, "Postorder called from within preorder or postorder" );
    }
     
    Target* target = nullptr;
    if ( !lua_isnoneornil(lua_state, TARGET) )
    {
        target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    }

    int bind_failures = graph->bind( target );
    if ( bind_failures > 0 )
    {
        lua_pushinteger( lua_state, bind_failures );
        return 1;
    }

    lua_pushvalue( lua_state, FUNCTION );
    int function = luaL_ref( lua_state, LUA_REGISTRYINDEX );
    int failures = forge->scheduler()->postorder( target, function );
    lua_pushinteger( lua_state, failures );
    luaL_unref( lua_state, LUA_REGISTRYINDEX, function );
    return 1;
}

int LuaGraph::print_dependencies( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TARGET = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    forge->graph()->print_dependencies( target, forge->context()->directory().string() );
    return 0;
}

int LuaGraph::print_namespace( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TARGET = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    forge->graph()->print_namespace( target );
    return 0;
}

int LuaGraph::wait( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    forge->scheduler()->wait();
    return 0;
}

int LuaGraph::prune( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    forge->scheduler()->prune();
    return 0;
}

int LuaGraph::clear( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    string working_directory = context->working_directory()->path();
    forge->graph()->clear();
    context->reset_directory( working_directory );
    return 0;
}

int LuaGraph::load_binary( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int FILENAME = 1;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    const char* filename = luaL_checkstring( lua_state, FILENAME );
    string working_directory = context->working_directory()->path();
    Target* cache_target = forge->graph()->load_binary( forge->absolute(string(filename)).string() );
    context->reset_directory( working_directory );
    if ( cache_target )
    {
        forge->create_target_lua_binding( cache_target );
    }
    luaxx_push( lua_state, cache_target );
    return 1;
}

int LuaGraph::save_binary( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    forge->graph()->save_binary();
    return 0;
}

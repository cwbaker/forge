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
#include <forge/Target.hpp>
#include <forge/TargetPrototype.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <algorithm>

using std::min;
using std::string;
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
        { "target_prototype", &LuaGraph::add_target_prototype },
        { "add_target_prototype", &LuaGraph::add_target_prototype },
        { "target", &LuaGraph::add_target },
        { "add_target", &LuaGraph::add_target },
        { "find_target", &LuaGraph::find_target },
        { "anonymous", &LuaGraph::anonymous },
        { "current_buildfile", &LuaGraph::current_buildfile },
        { "working_directory", &LuaGraph::working_directory },
        { "buildfile", &LuaGraph::buildfile },
        { "postorder", &LuaGraph::postorder },
        { "print_dependencies", &LuaGraph::print_dependencies },
        { "print_namespace", &LuaGraph::print_namespace },
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

int LuaGraph::add_target_prototype( lua_State* lua_state )
{
    try
    {
        const int FORGE = lua_upvalueindex( 1 );
        const int IDENTIFIER = 1;
        string id = luaL_checkstring( lua_state, IDENTIFIER );        
        Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
        TargetPrototype* target_prototype = forge->graph()->add_target_prototype( id );
        forge->create_target_prototype_lua_binding( target_prototype );
        luaxx_push( lua_state, target_prototype );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int LuaGraph::add_target( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TOOLSET = 1;
    const int IDENTIFIER = 2;
    const int TARGET_PROTOTYPE = 3;
    const int HASH = 4;
    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    Graph* graph = forge->graph();
    Target* working_directory = context->working_directory();
    size_t identifier_length = 0;
    const char* identifier = luaL_checklstring( lua_state, IDENTIFIER, &identifier_length );
    luaL_argcheck( lua_state, identifier && identifier_length > 0, IDENTIFIER, "missing or empty identifier" );
    const lua_Integer hash = luaL_optinteger( lua_state, HASH, 0 );
    TargetPrototype* target_prototype = (TargetPrototype*) luaxx_to( lua_state, TARGET_PROTOTYPE, TARGET_PROTOTYPE_TYPE );
    Target* target = graph->add_or_find_target( string(identifier, identifier_length), working_directory );

    bool update_target_prototype = target_prototype && !target->prototype();
    if ( update_target_prototype )
    {
        target->set_prototype( target_prototype );
        target->set_working_directory( working_directory );
    }

    bool update_working_directory = !target->working_directory();
    if ( update_working_directory )
    {
        target->set_working_directory( working_directory );
    }

    if ( target_prototype && target->prototype() != target_prototype )
    {
        forge->errorf( "The target '%s' has been created with prototypes '%s' and '%s'", identifier, target->prototype()->id().c_str(), target_prototype ? target_prototype->id().c_str() : "none" );
    }

    bool create_lua_binding = !target->referenced_by_script();
    if ( create_lua_binding )
    {
        forge->create_target_lua_binding( target );
    }

    // Set `target.forge` to the value of the Forge object that created 
    // this target.  The Forge object is used later on to provide the 
    // correct Forge object and settings when visiting targets in a 
    // postorder traversal.
    //
    // This also happens when the target prototype is set for the first time
    // so that targets that are lazily defined after they have been created by
    // another target depending on them have access to the Forge instance they
    // are defined in rather than just the first Forge instance that first 
    // referenced them which is difficult to control and typically incorrect.
    if ( update_target_prototype || update_working_directory || create_lua_binding )
    {
        luaxx_push( lua_state, target );
        lua_pushvalue( lua_state, TOOLSET );
        lua_setfield( lua_state, -2, "toolset" );
        // The `target.forge` field is set only for backwards compatibility 
        // while toolsets are added.
        lua_pushvalue( lua_state, TOOLSET );
        lua_setfield( lua_state, -2, "forge" );
        lua_pop( lua_state, 1 );
        target->set_hash( hash );
    }

    luaxx_push( lua_state, target );    
    return 1;
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

int LuaGraph::postorder( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TARGET = 1;
    const int FUNCTION = 2;

    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Graph* graph = forge->graph();
    if ( graph->traversal_in_progress() )
    {
        return luaL_error( lua_state, "Postorder called from within another bind or postorder traversal" );
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

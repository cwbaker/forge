//
// LuaGraph.cpp
// Copyright (c) 2015 Charles Baker. All rights reserved.
//

#include "LuaGraph.hpp"
#include "LuaBuildTool.hpp"
#include <sweet/build_tool/Context.hpp>
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/Scheduler.hpp>
#include <sweet/build_tool/Graph.hpp>
#include <sweet/build_tool/Target.hpp>
#include <sweet/build_tool/TargetPrototype.hpp>
#include <sweet/lua/LuaValue.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>
#include <algorithm>

using std::min;
using std::string;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

static const int BUILD_TOOL_UPVALUE = 1;

LuaGraph::LuaGraph()
{
}

LuaGraph::~LuaGraph()
{
    destroy();
}

void LuaGraph::create( BuildTool* build_tool, lua_State* lua_state )
{
    SWEET_ASSERT( build_tool );
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, -1) );

    destroy();

    static const luaL_Reg functions[] = 
    {
        { "target_prototype", &LuaGraph::target_prototype },
        { "file", &LuaGraph::file },
        { "target", &LuaGraph::target },
        { "find_target", &LuaGraph::find_target },
        { "anonymous", &LuaGraph::anonymous },
        { "working_directory", &LuaGraph::working_directory },
        { "buildfile", &LuaGraph::buildfile },
        { "postorder", &LuaGraph::postorder },
        { "print_dependencies", &LuaGraph::print_dependencies },
        { "print_namespace", &LuaGraph::print_namespace },
        { "wait", &LuaGraph::wait },
        { "clear", &LuaGraph::clear },
        { "load_xml", &LuaGraph::load_xml },
        { "save_xml", &LuaGraph::save_xml },
        { "load_binary", &LuaGraph::load_binary },
        { "save_binary", &LuaGraph::save_binary },
        { NULL, NULL }
    };
    lua_pushlightuserdata( lua_state, build_tool );
    luaL_setfuncs( lua_state, functions, 1 );
}

void LuaGraph::destroy()
{
}

Target* LuaGraph::add_target( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );

    Graph* graph = build_tool->graph();
    SWEET_ASSERT( graph );

    Target* working_directory = context->working_directory();
    SWEET_ASSERT( working_directory );

    const int ID = 1;
    bool anonymous = lua_isnoneornil( lua_state, ID );
    string id;
    if ( !anonymous ) 
    {
        id = luaL_checkstring( lua_state, ID );
    }
    else
    {
        char anonymous_id [256];
        unsigned int length = sprintf( anonymous_id, "$$%d", working_directory->next_anonymous_index() );
        id = string( anonymous_id, length );
        anonymous = true;
    }

    const int PROTOTYPE = 2;
    TargetPrototype* target_prototype = LuaConverter<TargetPrototype* >::to( lua_state, PROTOTYPE );

    Target* target = graph->target( id, target_prototype, working_directory );
    if ( !target->referenced_by_script() )
    {
        const int TABLE = 3;
        if ( !lua_isnoneornil(lua_state, TABLE) )
        {        
            luaL_argcheck( lua_state, lua_istable(lua_state, TABLE), TABLE, "Table or nothing expected as third parameter when creating a target" );
            
            lua_pushvalue( lua_state, TABLE );
            lua_create_object_with_existing_table( lua_state, target );
            target->set_referenced_by_script( true );
            target->set_prototype( target_prototype );
            target->set_working_directory( working_directory );
            build_tool->recover_target_lua_binding( target );
            build_tool->update_target_lua_binding( target );
        }
        else
        {
            build_tool->create_target_lua_binding( target );
            build_tool->recover_target_lua_binding( target );
            build_tool->update_target_lua_binding( target );
        }
    }
    return target;
}

int LuaGraph::target_prototype( lua_State* lua_state )
{
    try
    {
        const int ID = 1;
        string id;
        if ( lua_isstring(lua_state, ID) )
        {
            id = luaL_checkstring( lua_state, ID );
        }
        else 
        {
            const int TARGET_PROTOTYPE = 1;
            luaL_checktype( lua_state, TARGET_PROTOTYPE, LUA_TTABLE );
            
            lua_rawgeti( lua_state, TARGET_PROTOTYPE, 1 );
            if ( !lua_isstring(lua_state, -1) )
            {
                return luaL_error( lua_state, "TargetPrototype table expects a string to identify the prototype in the 1st field of the table" );
            }
            id = lua_tostring( lua_state, -1 );
            lua_pop( lua_state, 1 );
        }
        
        BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( build_tool );
        TargetPrototype* target_prototype = build_tool->graph()->target_prototype( id );
        SWEET_ASSERT( target_prototype );        
        lua_create_object( lua_state, target_prototype );
        build_tool->create_target_prototype_lua_binding( target_prototype );

        LuaConverter<TargetPrototype* >::push( lua_state, target_prototype );
        return 1;
    }
    
    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int LuaGraph::file( lua_State* lua_state )
{
    Target* target = LuaGraph::add_target( lua_state );
    target->set_filename( target->path() );
    LuaConverter<Target*>::push( lua_state, target );
    return 1;
}

int LuaGraph::target( lua_State* lua_state )
{
    Target* target = LuaGraph::add_target( lua_state );
    LuaConverter<Target*>::push( lua_state, target );
    return 1;
}

int LuaGraph::find_target( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );

    const int ID = 1;
    const char* id = luaL_checkstring( lua_state, ID );
    Target* target = build_tool->graph()->find_target( string(id), context->working_directory() );
    if ( target && !target->referenced_by_script() )
    {
        build_tool->create_target_lua_binding( target );
    }
    LuaConverter<Target*>::push( lua_state, target );
    return 1;
}

int LuaGraph::anonymous( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );

    Target* working_directory = context->working_directory();
    SWEET_ASSERT( working_directory );

    char anonymous [256];
    size_t length = sprintf( anonymous, "$$%d", working_directory->next_anonymous_index() );
    anonymous[min(length, sizeof(anonymous) - 1)] = 0;
    lua_pushlstring( lua_state, anonymous, length );
    return 1;
}

int LuaGraph::working_directory( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );
    
    Target* target = context->working_directory();
    if ( target && !target->referenced_by_script() )
    {
        build_tool->create_target_lua_binding( target );
    }
    LuaConverter<Target*>::push( lua_state, target );
    return 1;
}

int LuaGraph::buildfile( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );
    const int FILENAME = 1;
    const char* filename = luaL_checkstring( lua_state, FILENAME );
    build_tool->graph()->buildfile( string(filename) );
    return 0;
}

int LuaGraph::postorder( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Graph* graph = build_tool->graph();
    if ( graph->traversal_in_progress() )
    {
        return luaL_error( lua_state, "Postorder called from within another bind or postorder traversal" );
    }
     
    const int FUNCTION_PARAMETER = 1;
    LuaValue function( *build_tool->lua(), lua_state, FUNCTION_PARAMETER );

    const int TARGET_PARAMETER = 2;
    Target* target = NULL;
    if ( !lua_isnoneornil(lua_state, TARGET_PARAMETER) )
    {
        target = LuaConverter<Target*>::to( lua_state, TARGET_PARAMETER );
    }

    int bind_failures = graph->bind( target );
    if ( bind_failures > 0 )
    {
        lua_pushinteger( lua_state, bind_failures );
        return 1;
    }

    int failures = build_tool->scheduler()->postorder( function, target );
    lua_pushinteger( lua_state, failures );
    return 1;
}

int LuaGraph::print_dependencies( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );
    const int TARGET = 1;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    build_tool->graph()->print_dependencies( target, build_tool->context()->directory().string() );
    return 0;
}

int LuaGraph::print_namespace( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );
    const int TARGET = 1;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    build_tool->graph()->print_namespace( target );
    return 0;
}

int LuaGraph::wait( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );
    build_tool->scheduler()->wait();
    return 0;
}

int LuaGraph::clear( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );

    string working_directory = context->working_directory()->path();
    build_tool->graph()->clear();
    context->reset_directory( working_directory );
    return 0;
}

int LuaGraph::load_xml( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );

    const int FILENAME = 1;
    const char* filename = luaL_checkstring( lua_state, FILENAME );
    string working_directory = context->working_directory()->path();
    Target* cache_target = build_tool->graph()->load_xml( build_tool->absolute(string(filename)).string() );
    context->reset_directory( working_directory );
    if ( cache_target )
    {
        build_tool->create_target_lua_binding( cache_target );
    }
    LuaConverter<Target*>::push( lua_state, cache_target );
    return 1;
}

int LuaGraph::save_xml( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );
    build_tool->graph()->save_xml();
    return 0;
}

int LuaGraph::load_binary( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );

    Context* context = build_tool->context();
    SWEET_ASSERT( context );

    const int FILENAME = 1;
    const char* filename = luaL_checkstring( lua_state, FILENAME );
    string working_directory = context->working_directory()->path();
    Target* cache_target = build_tool->graph()->load_binary( build_tool->absolute(string(filename)).string() );
    context->reset_directory( working_directory );
    if ( cache_target )
    {
        build_tool->create_target_lua_binding( cache_target );
    }
    LuaConverter<Target*>::push( lua_state, cache_target );
    return 1;
}

int LuaGraph::save_binary( lua_State* lua_state )
{
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(BUILD_TOOL_UPVALUE)) );
    SWEET_ASSERT( build_tool );
    build_tool->graph()->save_binary();
    return 0;
}
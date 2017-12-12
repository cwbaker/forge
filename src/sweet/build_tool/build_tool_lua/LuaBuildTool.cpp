//
// LuaBuildTool.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaBuildTool.hpp"
#include "LuaFileSystem.hpp"
#include "LuaSystem.hpp"
#include "LuaContext.hpp"
#include "LuaGraph.hpp"
#include "LuaTargetPrototype.hpp"
#include "LuaTarget.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/Arguments.hpp>
#include <sweet/build_tool/Scheduler.hpp>
#include <sweet/process/Process.hpp>
#include <sweet/process/Environment.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/lua/LuaObject.hpp>
#include <sweet/lua/LuaValue.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>
#include <string>

using std::string;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

LuaBuildTool::LuaBuildTool( BuildTool* build_tool, lua::Lua* lua )
: build_( NULL ),
  lua_file_system_( NULL ),
  lua_context_( NULL ),
  lua_graph_( NULL ),
  lua_system_( NULL ),
  lua_target_( NULL ),
  lua_target_prototype_( NULL )
{
    create( build_tool, lua );
}

LuaBuildTool::~LuaBuildTool()
{
    destroy();
}

lua::LuaObject* LuaBuildTool::build() const
{
    SWEET_ASSERT( build_ );
    return build_;
}

LuaTarget* LuaBuildTool::lua_target() const
{
    SWEET_ASSERT( lua_target_ );
    return lua_target_;
}

LuaTargetPrototype* LuaBuildTool::lua_target_prototype() const
{
    SWEET_ASSERT( lua_target_prototype_ );
    return lua_target_prototype_;
}

void LuaBuildTool::create( BuildTool* build_tool, lua::Lua* lua )
{
    SWEET_ASSERT( build_tool );
    SWEET_ASSERT( lua );

    destroy();

    lua_file_system_ = new LuaFileSystem;
    lua_context_ = new LuaContext;
    lua_graph_ = new LuaGraph;
    lua_system_ = new LuaSystem;
    lua_target_ = new LuaTarget;
    lua_target_prototype_ = new LuaTargetPrototype;

    build_ = new LuaObject( *lua );
    build_->members()
        .type( SWEET_STATIC_TYPEID(BuildTool) )
        .this_pointer( build_tool )
        ( "set_maximum_parallel_jobs", &BuildTool::set_maximum_parallel_jobs )
        ( "maximum_parallel_jobs", &BuildTool::maximum_parallel_jobs )
        ( "set_stack_trace_enabled", &BuildTool::set_stack_trace_enabled )
        ( "stack_trace_enabled", &BuildTool::stack_trace_enabled )
        ( "set_build_hooks_library", &BuildTool::set_build_hooks_library )
        ( "build_hooks_library", &BuildTool::build_hooks_library )
        ( "execute", raw(&LuaBuildTool::execute) )
        ( "print", raw(&LuaBuildTool::print) )
    ;

    lua_State* lua_state = lua->get_lua_state();
    LuaConverter<LuaObject>::push( lua_state, *build_ );
    lua_context_->create( build_tool, lua_state );
    lua_file_system_->create( build_tool, lua_state );
    lua_graph_->create( build_tool, lua_state );
    lua_system_->create( build_tool, lua_state );
    lua_target_->create( lua );
    lua_target_prototype_->create( lua, lua_target_ );
    lua_pop( lua_state, 1 );

    lua->globals()
        ( "build", *build_ )
    ;
}

void LuaBuildTool::destroy()
{
    delete lua_target_prototype_;
    lua_target_prototype_ = NULL;

    delete lua_target_;
    lua_target_ = NULL;

    delete lua_system_;
    lua_system_ = NULL;

    delete lua_graph_;
    lua_graph_ = NULL;

    delete lua_context_;
    lua_context_ = NULL;

    delete lua_file_system_;
    lua_file_system_ = NULL;

    delete build_;
    build_ = NULL;
}

int LuaBuildTool::execute( lua_State* lua_state )
{
    try
    {
        const int BUILD_TOOL = 1;
        const int COMMAND = 2;
        const int COMMAND_LINE = 3;
        const int ENVIRONMENT = 4;
        const int DEPENDENCIES_FILTER = 5;
        const int STDOUT_FILTER = 6;
        const int STDERR_FILTER = 7;
        const int ARGUMENTS = 8;

        BuildTool* build_tool = LuaConverter<BuildTool*>::to( lua_state, BUILD_TOOL );
        const char* command = luaL_checkstring( lua_state, COMMAND );
        const char* command_line = luaL_checkstring( lua_state, COMMAND_LINE );
        unique_ptr<process::Environment> environment;
        if ( !lua_isnoneornil(lua_state, ENVIRONMENT) )
        {
            if ( !lua_istable(lua_state, ENVIRONMENT) )
            {
                lua_pushstring( lua_state, "Expected an environment table or nil as 3rd parameter" );
                return lua_error( lua_state );
            }
            
            environment.reset( new process::Environment );
            lua_pushnil( lua_state );
            while ( lua_next(lua_state, ENVIRONMENT) )
            {
                if ( lua_isstring(lua_state, -2) )
                {
                    const char* key = lua_tostring( lua_state, -2 );
                    const char* value = lua_tostring( lua_state, -1 );
                    environment->append( key, value );
                }
                lua_pop( lua_state, 1 );
            }
        }

        unique_ptr<lua::LuaValue> dependencies_filter;
        if ( !lua_isnoneornil(lua_state, DEPENDENCIES_FILTER) )
        {
            if ( !lua_isfunction(lua_state, DEPENDENCIES_FILTER) && !lua_istable(lua_state, DEPENDENCIES_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            dependencies_filter.reset( new lua::LuaValue(*build_tool->lua(), lua_state, DEPENDENCIES_FILTER) );
        }

        unique_ptr<lua::LuaValue> stdout_filter;
        if ( !lua_isnoneornil(lua_state, STDOUT_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDOUT_FILTER) && !lua_istable(lua_state, STDOUT_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 5th parameter (stdout filter)" );
                return lua_error( lua_state );
            }
            stdout_filter.reset( new lua::LuaValue(*build_tool->lua(), lua_state, STDOUT_FILTER) );
        }

        unique_ptr<lua::LuaValue> stderr_filter;
        if ( !lua_isnoneornil(lua_state, STDERR_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDERR_FILTER) && !lua_istable(lua_state, STDERR_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 6th parameter (stderr filter)" );
                return lua_error( lua_state );
            }
            stderr_filter.reset( new lua::LuaValue(*build_tool->lua(), lua_state, STDERR_FILTER) );
        }

        unique_ptr<Arguments> arguments;
        if ( lua_gettop(lua_state) >= ARGUMENTS )
        {
            arguments.reset( new Arguments(*build_tool->lua(), lua_state, ARGUMENTS, lua_gettop(lua_state) + 1) );
        }

        build_tool->scheduler()->execute( 
            string(command), 
            string(command_line), 
            environment.release(), 
            dependencies_filter.release(), 
            stdout_filter.release(), 
            stderr_filter.release(), 
            arguments.release(), 
            build_tool->context() 
        );

        return lua_yield( lua_state, 0 );
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );
    }
}

int LuaBuildTool::print( lua_State* lua_state )
{
    const int BUILD_TOOL = 1;
    const int TEXT = 2;
    BuildTool* build_tool = LuaConverter<BuildTool*>::to( lua_state, BUILD_TOOL );
    build_tool->output( luaL_checkstring(lua_state, TEXT) );
    return 0;
}

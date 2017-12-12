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
        ( "set_maximum_parallel_jobs", &BuildTool::set_maximum_parallel_jobs, build_tool )
        ( "maximum_parallel_jobs", &BuildTool::maximum_parallel_jobs, build_tool )
        ( "set_stack_trace_enabled", &BuildTool::set_stack_trace_enabled, build_tool )
        ( "stack_trace_enabled", &BuildTool::stack_trace_enabled, build_tool )
        ( "set_build_hooks_library", &BuildTool::set_build_hooks_library, build_tool )
        ( "build_hooks_library", &BuildTool::build_hooks_library, build_tool )
        ( "execute", raw(&LuaBuildTool::execute), build_tool )
        ( "print", raw(&LuaBuildTool::print), build_tool )
    ;

    lua_State* lua_state = lua->get_lua_state();
    LuaConverter<LuaObject>::push( lua_state, *build_ );
    lua_context_->create( build_tool, lua_state );
    lua_file_system_->create( lua_state );
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
        BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( build_tool );

        const int COMMAND = 1;
        const char* command = luaL_checkstring( lua_state, COMMAND );

        const int COMMAND_LINE = 2;
        const char* command_line = luaL_checkstring( lua_state, COMMAND_LINE );
        
        const int ENVIRONMENT = 3;
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

        const int DEPENDENCIES_FILTER = 4;
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

        const int STDOUT_FILTER = 5;
        unique_ptr<lua::LuaValue> stdout_filter;
        if ( !lua_isnoneornil(lua_state, STDOUT_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDOUT_FILTER) && !lua_istable(lua_state, STDOUT_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            stdout_filter.reset( new lua::LuaValue(*build_tool->lua(), lua_state, STDOUT_FILTER) );
        }

        const int STDERR_FILTER = 6;
        unique_ptr<lua::LuaValue> stderr_filter;
        if ( !lua_isnoneornil(lua_state, STDERR_FILTER) )
        {
            if ( !lua_isfunction(lua_state, STDERR_FILTER) && !lua_istable(lua_state, STDERR_FILTER) )
            {
                lua_pushstring( lua_state, "Expected a function or callable table as 4th parameter (dependencies filter)" );
                return lua_error( lua_state );
            }
            stderr_filter.reset( new lua::LuaValue(*build_tool->lua(), lua_state, STDERR_FILTER) );
        }

        const int ARGUMENTS = 7;
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
    BuildTool* build_tool = reinterpret_cast<BuildTool*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( build_tool );
    const int TEXT = 1;
    build_tool->output( luaL_checkstring(lua_state, TEXT) );
    return 0;
}

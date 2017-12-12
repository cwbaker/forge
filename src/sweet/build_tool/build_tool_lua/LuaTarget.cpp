//
// LuaTarget.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTarget.hpp"
#include "LuaBuildTool.hpp"
#include <sweet/build_tool/Target.hpp>
#include <sweet/build_tool/TargetPrototype.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/lua/LuaObject.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::build_tool;

LuaTarget::LuaTarget()
: lua_( NULL ),
  target_metatable_( NULL ),
  target_prototype_( NULL )
{
}

LuaTarget::~LuaTarget()
{
    destroy();
}

void LuaTarget::create( lua::Lua* lua )
{
    SWEET_ASSERT( lua );

    destroy();

    lua_ = lua;
    target_metatable_ = new lua::LuaObject( *lua_ );
    target_prototype_ = new lua::LuaObject( *lua_ );

    target_prototype_->members()
        .type( SWEET_STATIC_TYPEID(Target) )
    ;
    AddMember add_member = target_prototype_->members();
    register_functions( add_member );

    target_metatable_->members()
        ( "__index", *target_prototype_ )
        ( "__tostring", raw(&LuaTarget::filename) )
    ;
}

void LuaTarget::destroy()
{
    delete target_prototype_;
    target_prototype_ = NULL;

    delete target_metatable_;
    target_metatable_ = NULL;

    lua_ = NULL;
}

void LuaTarget::create_target( Target* target )
{
    SWEET_ASSERT( target );

    if ( !target->referenced_by_script() )
    {
        lua_->create( target );
        target->set_referenced_by_script( true );
        recover_target( target );
        update_target( target );
    }
}

void LuaTarget::recover_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_->members( target )
        .type( SWEET_STATIC_TYPEID(Target) )
        .metatable( *target_metatable_ )
        .this_pointer( target )
    ;
}

void LuaTarget::update_target( Target* target )
{
    SWEET_ASSERT( target );

    TargetPrototype* target_prototype = target->prototype();
    if ( target_prototype )
    {
        lua_->members( target )
            .metatable( target_prototype )
        ;        
    }
    else
    {
        lua_->members( target )
            .metatable( *target_metatable_ )
        ;
    }
}

void LuaTarget::destroy_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_->destroy( target );
    target->set_referenced_by_script( false );
}

void LuaTarget::register_functions( lua::AddMember& add_member )
{
    add_member
        ( "id", &Target::id )
        ( "path", &Target::path )
        ( "branch", &Target::branch )
        ( "parent", &LuaTarget::parent, this, _1 )
        ( "prototype", &Target::prototype )
        ( "set_required_to_exist", &Target::set_required_to_exist )
        ( "required_to_exist", &Target::required_to_exist )
        ( "set_cleanable", &Target::set_cleanable )
        ( "cleanable", &Target::cleanable )
        ( "set_built", &Target::set_built )
        ( "built", &Target::built )
        ( "timestamp", &Target::timestamp )
        ( "last_write_time", &Target::last_write_time )
        ( "outdated", &Target::outdated )
        ( "set_filename", raw(&LuaTarget::set_filename) )
        ( "filename", raw(&LuaTarget::filename) )
        ( "filenames", &Target::filenames )
        ( "directory", raw(&LuaTarget::directory) )
        ( "set_working_directory", &Target::set_working_directory )
        ( "working_directory", &LuaTarget::target_working_directory, this, _1 )
        ( "targets", raw(&LuaTarget::targets), this )
        ( "add_dependency", &Target::add_explicit_dependency )
        ( "remove_dependency", &Target::remove_dependency )
        ( "add_implicit_dependency", &Target::add_implicit_dependency )
        ( "clear_implicit_dependencies", &Target::clear_implicit_dependencies )
        ( "add_ordering_dependency", &Target::add_ordering_dependency )
        ( "dependency", raw(&LuaTarget::explicit_dependency), this )
        ( "dependencies", raw(&LuaTarget::explicit_dependencies), this )
        ( "implicit_dependency", raw(&LuaTarget::implicit_dependency), this )
        ( "implicit_dependencies", raw(&LuaTarget::implicit_dependencies), this )
        ( "ordering_dependency", raw(&LuaTarget::ordering_dependency), this )
        ( "ordering_dependencies", raw(&LuaTarget::ordering_dependencies), this )
        ( "any_dependency", raw(&LuaTarget::any_dependency), this )
        ( "any_dependencies", raw(&LuaTarget::any_dependencies), this )
    ;
}

Target* LuaTarget::parent( Target* target )
{
    SWEET_ASSERT( target );
    
    Target* parent = NULL;
    if ( target )
    {
        parent = target->parent();
        if ( parent && !parent->referenced_by_script() )
        {
            create_target( parent );
        }
    }
    return parent;
}

Target* LuaTarget::target_working_directory( Target* target )
{
    SWEET_ASSERT( target );
    
    Target* working_directory = NULL;
    if ( target )
    {
        working_directory = target->working_directory();
        if ( !working_directory->referenced_by_script() )
        {
            create_target( working_directory );
        }
    }
    return working_directory;
}

int LuaTarget::set_filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int FILENAME = 2;
    const int INDEX = 3;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    const char* filename = lua_tostring( lua_state, FILENAME );
    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;
    target->set_filename( string(filename), index );

    return 0;
}

int LuaTarget::filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    if ( index < int(target->filenames().size()) )
    {
        const string& filename = target->filename( index );
        lua_pushlstring( lua_state, filename.c_str(), filename.length() );
    }
    else
    {
        lua_pushlstring( lua_state, "", 0 );
    }
    return 1;
}

int LuaTarget::directory( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    if ( index < int(target->filenames().size()) )
    {
        string directory = target->directory( index );
        lua_pushlstring( lua_state, directory.c_str(), directory.length() );
    }
    else
    {
        lua_pushlstring( lua_state, "", 0 );
    }
    return 1;
}

struct GetTargetsTargetReferencedByScript
{
    LuaTarget* script_interface_;
    
    GetTargetsTargetReferencedByScript( LuaTarget* lua_target )
    : script_interface_( lua_target )
    {
        SWEET_ASSERT( script_interface_ );
    }
    
    bool operator()( lua_State* /*lua_state*/, Target* target ) const
    {
        SWEET_ASSERT( target );
        if ( !target->referenced_by_script() )
        {
            script_interface_->create_target( target );
        }
        return true;
    }
};

int LuaTarget::targets( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    try
    {
        LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( lua_target );

        const int TARGET = 1;
        Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
        luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

        const vector<Target*>& dependencies = target->targets();
        lua_push_iterator( lua_state, dependencies.begin(), dependencies.end(), GetTargetsTargetReferencedByScript(lua_target) );
        return 1;
    }

    catch ( const std::exception& exception )
    {
        lua_pushstring( lua_state, exception.what() );
        return lua_error( lua_state );        
    }
}

int LuaTarget::any_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->any_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        LuaConverter<Target*>::push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::any_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = lua_tointeger( lua_state, FINISH );
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        Target* dependency = target->any_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            LuaConverter<Target*>::push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::any_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target, TARGET, "expected target table" );

    int start = luaL_optinteger( lua_state, START, 1 );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = luaL_optinteger( lua_state, FINISH, INT_MAX );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::any_dependencies_iterator, 2 );
    LuaConverter<Target*>::push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::explicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->explicit_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        LuaConverter<Target*>::push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::explicit_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = lua_tointeger( lua_state, FINISH );
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        Target* dependency = target->explicit_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            LuaConverter<Target*>::push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::explicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int start = luaL_optinteger( lua_state, START, 1 );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = luaL_optinteger( lua_state, FINISH, INT_MAX );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::explicit_dependencies_iterator, 2 );
    LuaConverter<Target*>::push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::implicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->implicit_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        LuaConverter<Target*>::push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::implicit_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = lua_tointeger( lua_state, FINISH );
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        Target* dependency = target->implicit_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            LuaConverter<Target*>::push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::implicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int start = luaL_optinteger( lua_state, START, 1 );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = luaL_optinteger( lua_state, FINISH, INT_MAX );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::implicit_dependencies_iterator, 2 );
    LuaConverter<Target*>::push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::ordering_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
    luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
    --index;

    Target* dependency = target->ordering_dependency( index );
    if ( dependency )
    {
        if ( !dependency->referenced_by_script() )
        {
            LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( dependency );
        }
        LuaConverter<Target*>::push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::ordering_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = lua_tointeger( lua_state, FINISH );
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;
    if ( target && index <= finish )
    {
        Target* dependency = target->ordering_dependency( index - 1 );
        if ( dependency )
        {
            if ( !dependency->referenced_by_script() )
            {
                LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, LUA_TARGET) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( dependency );
            }
            lua_pushinteger( lua_state, index );
            LuaConverter<Target*>::push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::ordering_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;
    Target* target = LuaConverter<Target*>::to( lua_state, TARGET );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );
    int start = luaL_optinteger( lua_state, START, 1 );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );
    int finish = luaL_optinteger( lua_state, FINISH, INT_MAX );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::ordering_dependencies_iterator, 2 );
    LuaConverter<Target*>::push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

/**
// Push a std::time_t onto the Lua stack.
//
// @param lua_state
//  The lua_State to push the std::time_t onto the stack of.
//
// @param timestamp
//  The std::time_t to push.
//
// @relates Target
*/
void sweet::lua::lua_push( lua_State* lua_state, std::time_t timestamp )
{
    SWEET_ASSERT( lua_state );
    lua_pushnumber( lua_state, static_cast<lua_Number>(timestamp) );
}

/**
// Convert a number on the Lua stack into a std::time_t.
//
// @param lua_state
//  The lua_State to get the std::time_t from.
//
// @param position
//  The position of the std::time_t on the stack.
//
// @param null_pointer_for_overloading
//  Ignored.
//
// @return
//  The std::time_t.
//
// @relates Target
*/
std::time_t sweet::lua::lua_to( lua_State* lua_state, int position, const std::time_t* /*null_pointer_for_overloading*/ )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_isnumber(lua_state, position) );
    return static_cast<std::time_t>( lua_tonumber(lua_state, position) );
}

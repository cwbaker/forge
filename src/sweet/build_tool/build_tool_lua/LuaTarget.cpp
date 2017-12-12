//
// LuaTarget.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTarget.hpp"
#include "LuaBuildTool.hpp"
#include "types.hpp"
#include <sweet/build_tool/Target.hpp>
#include <sweet/build_tool/TargetPrototype.hpp>
#include <sweet/luaxx/luaxx.hpp>
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::lua;
using namespace sweet::luaxx;
using namespace sweet::build_tool;

static const char* TARGET_METATABLE = "build.Target";

LuaTarget::LuaTarget()
: lua_state_( nullptr )
{
}

LuaTarget::~LuaTarget()
{
    destroy();
}

void LuaTarget::create( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    destroy();

    lua_state_ = lua_state;
    luaxx_create( lua_state_, this, TARGET_TYPE );

    static const luaL_Reg functions[] = 
    {
        { "id", &LuaTarget::id },
        { "path", &LuaTarget::path },
        { "branch", &LuaTarget::branch },
        { "prototype", &LuaTarget::prototype },
        { "set_cleanable", &LuaTarget::set_cleanable },
        { "cleanable", &LuaTarget::cleanable },
        { "set_built", &LuaTarget::set_built },
        { "built", &LuaTarget::built },
        { "timestamp", &LuaTarget::timestamp },
        { "last_write_time", &LuaTarget::last_write_time },
        { "outdated", &LuaTarget::outdated },
        { "set_filename", &LuaTarget::set_filename },
        { "filename", &LuaTarget::filename },
        { "filenames", &LuaTarget::filenames },
        { "directory", &LuaTarget::directory },
        { "set_working_directory", &LuaTarget::set_working_directory },
        { "add_dependency", &LuaTarget::add_explicit_dependency },
        { "remove_dependency", &LuaTarget::remove_dependency },
        { "add_implicit_dependency", &LuaTarget::add_implicit_dependency },
        { "clear_implicit_dependencies", &LuaTarget::clear_implicit_dependencies },
        { "add_ordering_dependency", &LuaTarget::add_ordering_dependency },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_pop( lua_state_, 1 );

    static const luaL_Reg implicit_creation_functions [] = 
    {
        { "parent", &LuaTarget::parent },
        { "working_directory", &LuaTarget::working_directory },
        { "targets", &LuaTarget::targets },
        { "dependency", &LuaTarget::explicit_dependency },
        { "dependencies", &LuaTarget::explicit_dependencies },
        { "implicit_dependency", &LuaTarget::implicit_dependency },
        { "implicit_dependencies", &LuaTarget::implicit_dependencies },
        { "ordering_dependency", &LuaTarget::ordering_dependency },
        { "ordering_dependencies", &LuaTarget::ordering_dependencies },
        { "any_dependency", &LuaTarget::any_dependency },
        { "any_dependencies", &LuaTarget::any_dependencies },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    lua_pushlightuserdata( lua_state_, this );
    luaL_setfuncs( lua_state_, implicit_creation_functions, 1 );    
    lua_pop( lua_state_, 1 );

    luaL_newmetatable( lua_state_, TARGET_METATABLE );
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaTarget::filename );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pop( lua_state_, 1 );

    const int BUILD = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, BUILD, "Target" );
}

void LuaTarget::destroy()
{
    if ( lua_state_ )
    {
        luaxx_destroy( lua_state_, this );
        lua_state_ = nullptr;
    }
}

void LuaTarget::create_target( Target* target )
{
    SWEET_ASSERT( target );

    if ( !target->referenced_by_script() )
    {
        lua_create_object( lua_state_, target );
        target->set_referenced_by_script( true );
        recover_target( target );
        update_target( target );
    }
}

void LuaTarget::recover_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_push_object( lua_state_, target );
    lua_push_value<rtti::Type>( lua_state_, SWEET_STATIC_TYPEID(Target) );
    lua_setfield( lua_state_, -2, lua::TYPE_KEYWORD );
    lua_pushlightuserdata( lua_state_, target );
    lua_setfield( lua_state_, -2, lua::THIS_KEYWORD );
    luaL_setmetatable( lua_state_, TARGET_METATABLE );
    lua_pop( lua_state_, 1 );
}

void LuaTarget::update_target( Target* target )
{
    SWEET_ASSERT( target );

    TargetPrototype* target_prototype = target->prototype();
    if ( target_prototype )
    {
        lua_push_object( lua_state_, target );
        luaxx_push( lua_state_, target_prototype );
        lua_setmetatable( lua_state_, -2 );
        lua_pop( lua_state_, 1 );
    }
    else
    {
        lua_push_object( lua_state_, target );
        luaL_setmetatable( lua_state_, TARGET_METATABLE );
        lua_pop( lua_state_, 1 );
    }
}

void LuaTarget::destroy_target( Target* target )
{
    SWEET_ASSERT( target );
    lua_destroy_object( lua_state_, target );
    target->set_referenced_by_script( false );
}

int LuaTarget::id( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const string& id = target->id();
        lua_pushlstring( lua_state, id.c_str(), id.size() );
        return 1; 
    }
    return 0;
}

int LuaTarget::path( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const string& path = target->path();
        lua_pushlstring( lua_state, path.c_str(), path.size() );
        return 1; 
    }
    return 0;
}

int LuaTarget::branch( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const string& branch = target->branch();
        lua_pushlstring( lua_state, branch.c_str(), branch.size() );
        return 1; 
    }
    return 0;
}

int LuaTarget::parent( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* parent = target->parent();
        if ( parent && !parent->referenced_by_script() )
        {
            LuaTarget* lua_target = (LuaTarget*) lua_touserdata( lua_state, lua_upvalueindex(1) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( parent );
            lua_push_object( lua_state, parent );
            return 1;
        }
    }
    return 0;
}

int LuaTarget::prototype( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        TargetPrototype* target_prototype = target->prototype();
        luaxx_push( lua_state, target_prototype );
        return 1;
    }
    return 0;
}

int LuaTarget::set_cleanable( lua_State* lua_state )
{
    const int TARGET = 1;
    const int CLEANABLE = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        bool cleanable = lua_toboolean( lua_state, CLEANABLE );
        target->set_cleanable( cleanable );
    }
    return 0;
}

int LuaTarget::cleanable( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->cleanable() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::set_built( lua_State* lua_state )
{
    const int TARGET = 1;
    const int BUILT = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        bool built = lua_toboolean( lua_state, BUILT );
        target->set_built( built );
    }
    return 0;
}

int LuaTarget::built( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->built() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::timestamp( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushinteger( lua_state, target->timestamp() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::last_write_time( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushinteger( lua_state, target->last_write_time() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::outdated( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->outdated() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::set_filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int FILENAME = 2;
    const int INDEX = 3;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        size_t length = 0;
        const char* filename = luaL_checklstring( lua_state, FILENAME, &length );
        int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
        luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
        target->set_filename( string(filename, length), index - 1 );
    }
    return 0;
}

int LuaTarget::filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
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
    return 0;
}

int LuaTarget::filenames( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        const vector<string>& filenames = target->filenames();
        lua_push_value<vector<string>::const_iterator>( lua_state, filenames.begin() );
        lua_push_value<vector<string>::const_iterator>( lua_state, filenames.end() );
        lua_pushcclosure( lua_state, &lua_iterator<vector<string>::const_iterator>, 2 );
        return 1;
    }
    return 0;
}

int LuaTarget::directory( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        int index = lua_isnumber( lua_state, INDEX ) ? static_cast<int>( lua_tointeger(lua_state, INDEX) ) : 1;
        luaL_argcheck( lua_state, index >= 1, INDEX, "expected index >= 1" );
        --index;
        if ( index < int(target->filenames().size()) )
        {
            const string& directory = target->directory( index );
            lua_pushlstring( lua_state, directory.c_str(), directory.size() );
        }
        else
        {
            lua_pushlstring( lua_state, "", 0 );
        }
        return 1;
    }
    return 0;
}

int LuaTarget::set_working_directory( lua_State* lua_state )
{
    const int TARGET = 1;
    const int WORKING_DIRECTORY = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* working_directory = (Target*) lua_to_object( lua_state, WORKING_DIRECTORY, SWEET_STATIC_TYPEID(Target) );
        target->set_working_directory( working_directory );
    }
    return 0;
}

int LuaTarget::working_directory( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {    
        Target* working_directory = target->working_directory();
        if ( !working_directory->referenced_by_script() )
        {
            LuaTarget* lua_target = (LuaTarget*) lua_touserdata( lua_state, lua_upvalueindex(1) );
            SWEET_ASSERT( lua_target );
            lua_target->create_target( working_directory );
        }
        lua_push_object( lua_state, working_directory );
        return 1;
    }
    return 0;
}

int LuaTarget::add_explicit_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) lua_to_object( lua_state, DEPENDENCY, SWEET_STATIC_TYPEID(Target) );
        target->add_explicit_dependency( dependency );
    }
    return 0;
}

int LuaTarget::remove_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) lua_to_object( lua_state, DEPENDENCY, SWEET_STATIC_TYPEID(Target) );
        target->remove_dependency( dependency );
    }
    return 0;
}

int LuaTarget::add_implicit_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) lua_to_object( lua_state, DEPENDENCY, SWEET_STATIC_TYPEID(Target) );
        target->add_implicit_dependency( dependency );
    }
    return 0;
}

int LuaTarget::clear_implicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        target->clear_implicit_dependencies();
    }
    return 0;
}

int LuaTarget::add_ordering_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) lua_to_object( lua_state, DEPENDENCY, SWEET_STATIC_TYPEID(Target) );
        target->add_ordering_dependency( dependency );
    }
    return 0;
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
    const int TARGET = 1;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );
    if ( target )
    {
        LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( lua_target );
        const vector<Target*>& dependencies = target->targets();
        lua_push_iterator( lua_state, dependencies.begin(), dependencies.end(), GetTargetsTargetReferencedByScript(lua_target) );
        return 1;
    }
    return 0;
}

int LuaTarget::any_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
        lua_push_object( lua_state, dependency );
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
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
            lua_push_object( lua_state, dependency );
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

    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
    lua_push_object( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::explicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
        lua_push_object( lua_state, dependency );
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
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
            lua_push_object( lua_state, dependency );
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

    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
    lua_push_object( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::implicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
        lua_push_object( lua_state, dependency );
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
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
            lua_push_object( lua_state, dependency );
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

    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
    lua_push_object( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::ordering_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
        lua_push_object( lua_state, dependency );
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
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
            lua_push_object( lua_state, dependency );
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
    Target* target = (Target*) lua_to_object( lua_state, TARGET, SWEET_STATIC_TYPEID(Target) );
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
    lua_push_object( lua_state, target );
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

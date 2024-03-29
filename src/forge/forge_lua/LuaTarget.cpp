//
// LuaTarget.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "LuaTarget.hpp"
#include "LuaGraph.hpp"
#include "types.hpp"
#include <forge/Target.hpp>
#include <forge/Rule.hpp>
#include <forge/Context.hpp>
#include <forge/Forge.hpp>
#include <forge/Graph.hpp>
#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <algorithm>

using std::min;
using std::max;
using std::string;
using std::vector;
using namespace sweet;
using namespace sweet::luaxx;
using namespace sweet::forge;

static const char* STRING_VECTOR_CONST_ITERATOR_METATABLE = "forge.vector<string>::const_iterator";
const char* LuaTarget::TARGET_METATABLE = "forge.Target";

LuaTarget::LuaTarget()
: lua_state_( nullptr )
{
}

LuaTarget::~LuaTarget()
{
    destroy();
}

void LuaTarget::create( lua_State* lua_state, Forge* forge )
{
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( forge );

    destroy();

    lua_state_ = lua_state;
    luaxx_create( lua_state_, this, TARGET_TYPE );

    static const luaL_Reg functions[] = 
    {
        { "id", &LuaTarget::id },
        { "path", &LuaTarget::path },
        { "branch", &LuaTarget::branch },
        { "rule", &LuaTarget::rule },
        { "set_cleanable", &LuaTarget::set_cleanable },
        { "cleanable", &LuaTarget::cleanable },
        { "set_built", &LuaTarget::set_built },
        { "built", &LuaTarget::built },
        { "timestamp", &LuaTarget::timestamp },
        { "last_write_time", &LuaTarget::last_write_time },
        { "outdated", &LuaTarget::outdated },
        { "add_filename", &LuaTarget::add_filename },
        { "set_filename", &LuaTarget::set_filename },
        { "clear_filenames", &LuaTarget::clear_filenames },
        { "filename", &LuaTarget::filename },
        { "filenames", &LuaTarget::filenames },
        { "directory", &LuaTarget::directory },
        { "set_working_directory", &LuaTarget::set_working_directory },
        { "add_dependency", &LuaTarget::add_explicit_dependency },
        { "remove_dependency", &LuaTarget::remove_dependency },
        { "add_implicit_dependency", &LuaTarget::add_implicit_dependency },
        { "clear_implicit_dependencies", &LuaTarget::clear_implicit_dependencies },
        { "add_ordering_dependency", &LuaTarget::add_ordering_dependency },
        { "add_passive_dependency", &LuaTarget::add_passive_dependency },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    luaL_setfuncs( lua_state_, functions, 0 );
    lua_pop( lua_state_, 1 );

    static const luaL_Reg implicit_creation_functions [] = 
    {
        { "parent", &LuaTarget::parent },
        { "working_directory", &LuaTarget::working_directory },
        { "dependency", &LuaTarget::explicit_dependency },
        { "dependencies", &LuaTarget::explicit_dependencies },
        { "ordering_dependency", &LuaTarget::ordering_dependency },
        { "all_dependencies", &LuaTarget::all_dependencies },
        { nullptr, nullptr }
    };
    luaxx_push( lua_state_, this );
    lua_pushlightuserdata( lua_state_, this );
    luaL_setfuncs( lua_state_, implicit_creation_functions, 1 );    
    lua_pop( lua_state_, 1 );

    // Set the metatable for `Target` to redirect calls to create new targets.
    luaxx_push( lua_state_, this );
    lua_newtable( lua_state_ );
    lua_pushlightuserdata( lua_state, forge );
    lua_pushcclosure( lua_state_, &LuaTarget::target_call_metamethod, 1 );
    lua_setfield( lua_state_, -2, "__call" );
    lua_setmetatable( lua_state_, -2 );
    lua_pop( lua_state_, 1 );

    // Create a metatable for targets to redirect index operations to 
    // `forge.Target`, string conversions to `LuaTarget::filename()`, and
    // calls to `Target.depend()` via `LuaTarget::depend_call_metamethod()`.
    luaL_newmetatable( lua_state_, TARGET_METATABLE );
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, -2, "__index" );
    lua_pushcfunction( lua_state_, &LuaTarget::filename );
    lua_setfield( lua_state_, -2, "__tostring" );
    lua_pushcfunction( lua_state_, &LuaTarget::depend_call_metamethod );
    lua_setfield( lua_state_, -2, "__call" );
    lua_pop( lua_state_, 1 );

    // Create a metatable to garbage collect the string vector iterator used
    // to implement `Target.filenames()` via `LuaTarget::filenames()`.
    luaL_newmetatable( lua_state_, STRING_VECTOR_CONST_ITERATOR_METATABLE );
    lua_pushcfunction( lua_state_, &vector_string_const_iterator_gc );
    lua_setfield( lua_state_, -2, "__gc" );
    lua_pop( lua_state_, 1 );

    // Set `forge.Target` to this object.
    const int FORGE = 1;
    luaxx_push( lua_state_, this );
    lua_setfield( lua_state_, FORGE, "Target" );

    // Set global `Target` to this object.
    luaxx_push( lua_state_, this );
    lua_setglobal( lua_state_, "Target" );
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
        luaxx_create( lua_state_, target, TARGET_TYPE );
        target->set_referenced_by_script( true );
        update_target( target );
    }
}

void LuaTarget::update_target( Target* target )
{
    SWEET_ASSERT( target );
    Rule* rule = target->rule();
    if ( rule )
    {
        luaxx_push( lua_state_, target );
        luaxx_push( lua_state_, rule );
        lua_setmetatable( lua_state_, -2 );
        lua_pop( lua_state_, 1 );
    }
    else
    {
        luaxx_push( lua_state_, target );
        luaL_setmetatable( lua_state_, TARGET_METATABLE );
        lua_pop( lua_state_, 1 );
    }
}

void LuaTarget::destroy_target( Target* target )
{
    SWEET_ASSERT( target );
    luaxx_destroy( lua_state_, target );
    target->set_referenced_by_script( false );
}

int LuaTarget::id( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* parent = target->parent();
        if ( parent )
        {
            if ( !parent->referenced_by_script() )
            {
                LuaTarget* lua_target = (LuaTarget*) lua_touserdata( lua_state, lua_upvalueindex(1) );
                SWEET_ASSERT( lua_target );
                lua_target->create_target( parent );
            }
            luaxx_push( lua_state, parent );
            return 1;
        }
    }
    return 0;
}

int LuaTarget::rule( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Rule* rule = target->rule();
        luaxx_push( lua_state, rule );
        return 1;
    }
    return 0;
}

int LuaTarget::set_cleanable( lua_State* lua_state )
{
    const int TARGET = 1;
    const int CLEANABLE = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        bool cleanable = lua_toboolean( lua_state, CLEANABLE ) != 0;
        target->set_cleanable( cleanable );
    }
    return 0;
}

int LuaTarget::cleanable( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        bool built = lua_toboolean( lua_state, BUILT ) != 0;
        target->set_built( built );
    }
    return 0;
}

int LuaTarget::built( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        lua_pushboolean( lua_state, target->outdated() ? 1 : 0 );
        return 1;
    }
    return 0;
}

int LuaTarget::add_filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int FILENAME = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        size_t length = 0;
        const char* filename = luaL_checklstring( lua_state, FILENAME, &length );
        target->add_filename( string(filename, length) );
    }
    return 0;
}

int LuaTarget::set_filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int FILENAME = 2;
    const int INDEX = 3;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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

int LuaTarget::clear_filenames( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    int start = (int) luaL_optinteger( lua_state, START, 0 );
    int finish = (int) luaL_optinteger( lua_state, FINISH, INT_MAX );
    if ( target )
    {
        target->clear_filenames( start, finish );
    }
    return 0;
}

int LuaTarget::filename( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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

int LuaTarget::filenames_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    int index = static_cast<int>( lua_tointeger(lua_state, INDEX) ) + 1;

    if ( target && index <= finish )
    {
        SWEET_ASSERT( index <= int(target->filenames().size()) );
        const string& filename = target->filename( index - 1 );
        lua_pushinteger( lua_state, index );
        lua_pushlstring( lua_state, filename.c_str(), filename.length() );
        return 2;
    }
    return 0;
}

int LuaTarget::filenames( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );

    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );   

    const vector<string>& filenames = target->filenames();
    finish = min( finish, int(filenames.size()) );

    lua_pushinteger( lua_state, finish );
    lua_pushcclosure( lua_state, &LuaTarget::filenames_iterator, 1 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::directory( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* working_directory = (Target*) luaxx_to( lua_state, WORKING_DIRECTORY, TARGET_TYPE );
        target->set_working_directory( working_directory );
    }
    return 0;
}

int LuaTarget::working_directory( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
        luaxx_push( lua_state, working_directory );
        return 1;
    }
    return 0;
}

int LuaTarget::add_explicit_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_explicit_dependency( dependency );
    }
    return 0;
}

int LuaTarget::remove_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->remove_dependency( dependency );
    }
    return 0;
}

int LuaTarget::add_implicit_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_implicit_dependency( dependency );
    }
    return 0;
}

int LuaTarget::clear_implicit_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_ordering_dependency( dependency );
    }
    return 0;
}

int LuaTarget::add_passive_dependency( lua_State* lua_state )
{
    const int TARGET = 1;
    const int DEPENDENCY = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "nil target" );
    if ( target )
    {
        Target* dependency = (Target*) luaxx_to( lua_state, DEPENDENCY, TARGET_TYPE );
        target->add_passive_dependency( dependency );
    }
    return 0;
}

int LuaTarget::all_dependencies_iterator( lua_State* lua_state )
{
    const int TARGET = 1;
    const int INDEX = 2;
    const int FINISH = lua_upvalueindex( 1 );
    const int LUA_TARGET = lua_upvalueindex( 2 );

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
            luaxx_push( lua_state, dependency );
            return 2;
        }
    }
    return 0;
}

int LuaTarget::all_dependencies( lua_State* lua_state )
{
    const int TARGET = 1;
    const int START = 2;
    const int FINISH = 3;

    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target, TARGET, "expected target table" );

    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::all_dependencies_iterator, 2 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::explicit_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );

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
        luaxx_push( lua_state, dependency );
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

    int finish = static_cast<int>( lua_tointeger(lua_state, FINISH) );
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
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
            luaxx_push( lua_state, dependency );
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

    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != nullptr, TARGET, "expected target table" );

    int start = static_cast<int>( luaL_optinteger(lua_state, START, 1) );
    luaL_argcheck( lua_state, start >= 1, START, "expected start >= 1" );

    int finish = static_cast<int>( luaL_optinteger(lua_state, FINISH, INT_MAX) );
    luaL_argcheck( lua_state, finish >= start, FINISH, "expected finish >= start" );

    LuaTarget* lua_target = reinterpret_cast<LuaTarget*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( lua_target );    
    lua_pushinteger( lua_state, finish );
    lua_pushlightuserdata( lua_state, lua_target );
    lua_pushcclosure( lua_state, &LuaTarget::explicit_dependencies_iterator, 2 );
    luaxx_push( lua_state, target );
    lua_pushinteger( lua_state, start - 1 );
    return 3;
}

int LuaTarget::ordering_dependency( lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );

    const int TARGET = 1;
    const int INDEX = 2;
    Target* target = (Target*) luaxx_to( lua_state, TARGET, TARGET_TYPE );
    luaL_argcheck( lua_state, target != NULL, TARGET, "expected target table" );

    int index = int( luaL_optinteger(lua_state, INDEX, 1) );
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
        luaxx_push( lua_state, dependency );
    }
    else
    {
        lua_pushnil( lua_state );
    }
    return 1;
}

int LuaTarget::vector_string_const_iterator_gc( lua_State* lua_state )
{
    return luaxx_gc<vector<string>::const_iterator>( lua_state );
}

/**
// Create a new target (as metamethod for calls made on `Target`).
*/
int LuaTarget::target_call_metamethod( lua_State* lua_state )
{
    const int FORGE = lua_upvalueindex( 1 );
    const int TARGET = 1;
    const int TOOLSET = 2;
    const int IDENTIFIER = 3;
    const int RULE = 4;

    // Ignore `Target` passed as first parameter.
    (void) TARGET;

    Forge* forge = (Forge*) lua_touserdata( lua_state, FORGE );
    Context* context = forge->context();
    Graph* graph = forge->graph();
    Target* working_directory = context->working_directory();
    size_t identifier_length = 0;
    const char* identifier = luaL_checklstring( lua_state, IDENTIFIER, &identifier_length );
    Rule* rule = (Rule*) luaxx_to( lua_state, RULE, RULE_TYPE );

    Target* target = graph->add_or_find_target( string(identifier, identifier_length), working_directory );

    bool update_rule = rule && !target->rule();
    if ( update_rule )
    {
        target->set_rule( rule );
        target->set_working_directory( working_directory );
    }

    bool update_working_directory = !target->working_directory();
    if ( update_working_directory )
    {
        target->set_working_directory( working_directory );
    }

    if ( rule && target->rule() != rule )
    {
        forge->errorf( "The target '%s' has been created with rules '%s' and '%s'", identifier, target->rule()->id().c_str(), rule ? rule->id().c_str() : "none" );
    }

    bool create_lua_binding = !target->referenced_by_script();
    if ( create_lua_binding )
    {
        forge->create_target_lua_binding( target );
    }

    // Set `target.toolset` to the toolset that created this target.  The
    // toolset is used to provide the correct toolset and settings when
    // visiting targets in a postorder traversal.
    //
    // This also happens when the target rule is set for the first time
    // so that targets that are lazily defined after they have been created by
    // another target depending on them have access to the Forge instance they
    // are defined in rather than just the first Forge instance that first 
    // referenced them which is difficult to control and typically incorrect.
    if ( update_rule || update_working_directory || create_lua_binding )
    {
        luaxx_push( lua_state, target );
        lua_pushvalue( lua_state, TOOLSET );
        lua_setfield( lua_state, -2, "toolset" );
        lua_pop( lua_state, 1 );

        lua_getglobal( lua_state, "hash" );
        lua_pushvalue( lua_state, TOOLSET );
        if ( lua_istable(lua_state, -1) )
        {
            lua_call( lua_state, 1, 1 );
            const lua_Integer hash = luaL_checkinteger( lua_state, -1 );
            target->set_hash( hash );
        }
        lua_pop( lua_state, 1 );

        // The following calculation of the hash of the settings table without
        // calling into the Lua API so much should work but crashes for some
        // reason I don't understand.
        // lua_pushvalue( lua_state, TOOLSET );
        // lua_Integer hash = LuaSystem::hash_recursively( lua_state, -1, false );
        // lua_pop( lua_state, 1 );
    }

    luaxx_push( lua_state, target );    
    return 1;
}

/**
// Redirect calls made on target objects to `Target.depend()`.
//
// The call to `Target.depend()` may be overridden by providing `depend()`
// methods on `forge.Target`, individual target prototypes, and even 
// individual targets if necessary.
//
// ~~~lua
// function depend_call_metamethod( target, ... )
//     local depend_function = target.depend;
//     depend_function( target.toolset, target, ... );
//     return target;
// end
// ~~~
*/
int LuaTarget::depend_call_metamethod( lua_State* lua_state )
{
    const int TARGET = 1;
    const int VARARGS = 2;
    int args = lua_gettop( lua_state );
    lua_getfield( lua_state, TARGET, "depend" );
    lua_getfield( lua_state, TARGET, "toolset" );
    lua_pushvalue( lua_state, TARGET );
    for ( int i = VARARGS; i <= args; ++i )
    {
        lua_pushvalue( lua_state, i );
    }
    lua_call( lua_state, args + 1, 0 );
    lua_settop( lua_state, TARGET );
    return 1;
}

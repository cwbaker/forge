//
// LuaFs.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "LuaFs.hpp"
#include <sweet/fs/BasicPath.ipp>
#include <sweet/fs/DirectoryStack.hpp>
#include <sweet/fs/FileSystem.hpp>
#include <sweet/fs/fs.hpp>
#include <lua/lua.hpp>

using std::string;
using boost::filesystem::directory_iterator;
using boost::filesystem::recursive_directory_iterator;
using namespace sweet;
using namespace sweet::fs;

static const char* DIRECTORY_ITERATOR_METATABLE = "boost::filesystem::directory_iterator";
static const char* RECURSIVE_DIRECTORY_ITERATOR_METATABLE = "boost::filesystem::recursive_directory_iterator";

/**
// Constructor.
*/
LuaFs::LuaFs()
: lua_state_( NULL )
{
}

/**
// Destructor.
*/
LuaFs::~LuaFs()
{
    destroy();
}

/**
// Add the LuaFs API functions into the table at the top of the stack.
//
// Registers the functions that make up the API into a table as closures
// with 'file_system' as their first upvalue.  The table is left on the 
// stack for manipulation by the caller (e.g. to call lua_global() or 
// otherwise store a reference for later use).
//
// @param file_system
//  The FileSystem to use as underlying context for function calls like
//  'root()', 'initial()', etc and from which to retrieve the DirectoryStack
//  for calls to functions like 'cd()', 'pushd()', 'popd()', etc (assumed
//  not null).
//
// @param lua_state
//  The lua_State to create the LuaFs API within (assumed not null).
*/
void LuaFs::create( FileSystem* file_system, lua_State* lua_state )
{
    SWEET_ASSERT( file_system );
    SWEET_ASSERT( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, -1) );

    const luaL_Reg file_system_functions[] = 
    {
        { "cd", &LuaFs::cd },
        { "pushd", &LuaFs::pushd },
        { "popd", &LuaFs::popd },
        { "pwd", &LuaFs::pwd },
        { "absolute", &LuaFs::absolute },
        { "relative", &LuaFs::relative },
        { "root", &LuaFs::root },
        { "initial", &LuaFs::initial },
        { "executable", &LuaFs::executable },
        { "home", &LuaFs::home },
        { NULL, NULL }
    };
    lua_pushlightuserdata( lua_state, file_system );
    luaL_setfuncs( lua_state, file_system_functions, 1 );

    static const luaL_Reg functions[] = 
    {
        { "native", &LuaFs::native },
        { "branch", &LuaFs::branch },
        { "leaf", &LuaFs::leaf },
        { "basename", &LuaFs::basename },
        { "extension", &LuaFs::extension },
        { "is_absolute", &LuaFs::is_absolute },
        { "is_relative", &LuaFs::is_relative },
        { "exists", &LuaFs::exists },
        { "is_file", &LuaFs::is_file },
        { "is_directory", &LuaFs::is_directory },
        { "ls", &LuaFs::ls },
        { "find", &LuaFs::find },
        { "mkdir", &LuaFs::mkdir },
        { "rmdir", &LuaFs::rmdir },
        { "cp", &LuaFs::cp },
        { "rm", &LuaFs::rm },
        { NULL, NULL }
    };
    luaL_setfuncs( lua_state, functions, 0 );

    luaL_newmetatable( lua_state, DIRECTORY_ITERATOR_METATABLE );
    lua_pushstring( lua_state, "__gc" );
    lua_pushcfunction( lua_state, &LuaFs::directory_iterator_gc );
    lua_rawset( lua_state, -3 );
    lua_pop( lua_state, 1 );

    luaL_newmetatable( lua_state, RECURSIVE_DIRECTORY_ITERATOR_METATABLE );
    lua_pushstring( lua_state, "__gc" );
    lua_pushcfunction( lua_state, &LuaFs::recursive_directory_iterator_gc );
    lua_rawset( lua_state, -3 );
    lua_pop( lua_state, 1 );
}

/**
// Set the value of the global 'fs' back to nil. 
*/
void LuaFs::destroy()
{
    if ( lua_state_ )
    {
        lua_pushstring( lua_state_, DIRECTORY_ITERATOR_METATABLE );
        lua_pushnil( lua_state_ );
        lua_rawset( lua_state_, LUA_REGISTRYINDEX );

        lua_pushstring( lua_state_, RECURSIVE_DIRECTORY_ITERATOR_METATABLE );
        lua_pushnil( lua_state_ );
        lua_rawset( lua_state_, LUA_REGISTRYINDEX );

        lua_state_ = NULL;
    }
}

int LuaFs::cd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    file_system->change_directory( fs::Path(string(path, length)) );
    return 0;
}

int LuaFs::pushd( lua_State* lua_state )
{
    const int PATH = 1;
    size_t length = 0;
    const char* path = luaL_checklstring( lua_state, PATH, &length );
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    file_system->push_directory( fs::Path(string(path, length)) );
    return 0;
}

int LuaFs::popd( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    file_system->pop_directory();
    return 0;
}

int LuaFs::pwd( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );
    const fs::Path& path = file_system->directory();
    lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    return 1;
}

int LuaFs::absolute( lua_State* lua_state )
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
        FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( file_system );
        DirectoryStack* directory_stack = file_system->directory_stack();
        SWEET_ASSERT( directory_stack );
        const fs::Path& base_path = directory_stack->directory();
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaFs::relative( lua_State* lua_state )
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
        FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
        SWEET_ASSERT( file_system );
        DirectoryStack* directory_stack = file_system->directory_stack();
        SWEET_ASSERT( directory_stack );
        const fs::Path& base_path = directory_stack->directory();
        fs::Path path = fs::absolute( fs::Path(luaL_checkstring(lua_state, PATH)), base_path );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    return 1;
}

int LuaFs::root( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->root();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->root( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::initial( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->initial();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->initial( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::executable( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->executable();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->executable( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::home( lua_State* lua_state )
{
    FileSystem* file_system = reinterpret_cast<FileSystem*>( lua_touserdata(lua_state, lua_upvalueindex(1)) );
    SWEET_ASSERT( file_system );

    const int PATH = 1;
    if ( lua_isnoneornil(lua_state, PATH) )
    {
        const fs::Path& path = file_system->home();
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }
    else
    {
        fs::Path path = file_system->home( fs::Path(luaL_checkstring(lua_state, PATH)) );
        lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    }

    return 1;
}

int LuaFs::native( lua_State* lua_state )
{
    const int PATH = 1;
    string native_string = fs::Path( luaL_checkstring(lua_state, PATH) ).native_string();
    lua_pushlstring( lua_state, native_string.c_str(), native_string.length() );
    return 1;
}

int LuaFs::branch( lua_State* lua_state )
{
    const int PATH = 1;
    fs::Path path = fs::Path( luaL_checkstring(lua_state, PATH) ).branch();
    lua_pushlstring( lua_state, path.string().c_str(), path.string().length() );
    return 1;
}

int LuaFs::leaf( lua_State* lua_state )
{
    const int PATH = 1;
    string leaf = fs::Path( luaL_checkstring(lua_state, PATH) ).leaf();
    lua_pushlstring( lua_state, leaf.c_str(), leaf.length() );
    return 1;
}

int LuaFs::basename( lua_State* lua_state )
{
    const int PATH = 1;
    string basename = fs::Path( luaL_checkstring(lua_state, PATH) ).basename();
    lua_pushlstring( lua_state, basename.c_str(), basename.length() );
    return 1;
}

int LuaFs::extension( lua_State* lua_state )
{
    const int PATH = 1;
    string extension = fs::Path( luaL_checkstring(lua_state, PATH) ).extension();
    lua_pushlstring( lua_state, extension.c_str(), extension.length() );
    return 1;
}

int LuaFs::is_absolute( lua_State* lua_state )
{
    const int PATH = 1;
    bool absolute = fs::Path( luaL_checkstring(lua_state, PATH) ).is_absolute();
    lua_pushboolean( lua_state, absolute ? 1 : 0 );
    return 1;
}

int LuaFs::is_relative( lua_State* lua_state )
{
    const int PATH = 1;
    bool relative = fs::Path( luaL_checkstring(lua_state, PATH) ).is_relative();
    lua_pushboolean( lua_state, relative ? 1 : 0 );
    return 1;
}

int LuaFs::exists( lua_State* lua_state )
{
    const int PATH = 1;
    const char* path = luaL_checkstring( lua_state, PATH );
    lua_pushboolean( lua_state, boost::filesystem::exists(path) ? 1 : 0 );
    return 1;
}

int LuaFs::is_file( lua_State* lua_state )
{
    const int PATH = 1;
    const char* path = luaL_checkstring( lua_state, PATH );
    lua_pushboolean( lua_state, boost::filesystem::is_regular_file(path) ? 1 : 0 );
    return 1;
}

int LuaFs::is_directory( lua_State* lua_state )
{
    const int PATH = 1;
    const char* path = luaL_checkstring( lua_state, PATH );
    lua_pushboolean( lua_state, boost::filesystem::is_directory(path) ? 1 : 0 );
    return 1;
}

int LuaFs::ls( lua_State* lua_state )
{
    const int PATH = 1;
    const char* path = luaL_checkstring( lua_state, PATH );
    LuaFs::push_directory_iterator( lua_state, directory_iterator(path) );
    lua_pushcclosure( lua_state, &LuaFs::ls_iterator, 1 );
    return 1;
}

int LuaFs::find( lua_State* lua_state )
{
    const int PATH = 1;
    const char* path = luaL_checkstring( lua_state, PATH );
    LuaFs::push_recursive_directory_iterator( lua_state, recursive_directory_iterator(path) );
    lua_pushcclosure( lua_state, &LuaFs::ls_iterator, 1 );
    return 1;
}

int LuaFs::mkdir( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::create_directories( luaL_checkstring(lua_state, PATH) );
    return 0;
}

int LuaFs::rmdir( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::remove_all( luaL_checkstring(lua_state, PATH) );
    return 0;
}

int LuaFs::cp( lua_State* lua_state )
{
    const int FROM = 1;
    const int TO = 2;
    boost::filesystem::copy_file( luaL_checkstring(lua_state, FROM), luaL_checkstring(lua_state, TO) );
    return 0;
}

int LuaFs::rm( lua_State* lua_state )
{
    const int PATH = 1;
    boost::filesystem::remove( luaL_checkstring(lua_state, PATH) );
    return 0;
}

int LuaFs::ls_iterator( lua_State* lua_state )
{
    directory_iterator& iterator = *LuaFs::to_directory_iterator( lua_state, lua_upvalueindex(1) );
    directory_iterator end;
    if ( iterator != end )
    {
        const boost::filesystem::directory_entry& entry = *iterator;
        lua_pushlstring( lua_state, entry.path().string().c_str(), entry.path().string().length() );
        ++iterator;
        return 1;
    }
    return 0;
}

void LuaFs::push_directory_iterator( lua_State* lua_state, const boost::filesystem::directory_iterator& iterator )
{
    directory_iterator* other_iterator = (directory_iterator*) lua_newuserdata( lua_state, sizeof(directory_iterator) );
    new (other_iterator) directory_iterator( iterator );
    luaL_getmetatable( lua_state, DIRECTORY_ITERATOR_METATABLE );
    lua_setmetatable( lua_state, -2 );
}

boost::filesystem::directory_iterator* LuaFs::to_directory_iterator( lua_State* lua_state, int index )
{
    directory_iterator* iterator = (directory_iterator*) luaL_checkudata( lua_state, index, DIRECTORY_ITERATOR_METATABLE );
    luaL_argcheck( lua_state, iterator != NULL, index, "directory iterator expected" );
    return iterator;
}

int LuaFs::directory_iterator_gc( lua_State* lua_state )
{
    const int ITERATOR = 1;
    const directory_iterator* iterator = LuaFs::to_directory_iterator( lua_state, ITERATOR );
    iterator->~directory_iterator();
    return 0;
}

int LuaFs::find_iterator( lua_State* lua_state )
{
    recursive_directory_iterator& iterator = *LuaFs::to_recursive_directory_iterator( lua_state, lua_upvalueindex(1) );
    recursive_directory_iterator end;
    if ( iterator != end )
    {
        const boost::filesystem::directory_entry& entry = *iterator;
        lua_pushlstring( lua_state, entry.path().string().c_str(), entry.path().string().length() );
        ++iterator;
        return 1;
    }
    return 0;
}

void LuaFs::push_recursive_directory_iterator( lua_State* lua_state, const boost::filesystem::recursive_directory_iterator& iterator )
{
    recursive_directory_iterator* other_iterator = (recursive_directory_iterator*) lua_newuserdata( lua_state, sizeof(recursive_directory_iterator) );
    new (other_iterator) recursive_directory_iterator( iterator );
    luaL_getmetatable( lua_state, DIRECTORY_ITERATOR_METATABLE );
    lua_setmetatable( lua_state, -2 );
}

boost::filesystem::recursive_directory_iterator* LuaFs::to_recursive_directory_iterator( lua_State* lua_state, int index )
{
    recursive_directory_iterator* iterator = (recursive_directory_iterator*) luaL_checkudata( lua_state, index, DIRECTORY_ITERATOR_METATABLE );
    luaL_argcheck( lua_state, iterator != NULL, index, "directory iterator expected" );
    return iterator;
}

int LuaFs::recursive_directory_iterator_gc( lua_State* lua_state )
{
    const int ITERATOR = 1;
    const recursive_directory_iterator* iterator = LuaFs::to_recursive_directory_iterator( lua_state, ITERATOR );
    iterator->~recursive_directory_iterator();
    return 0;
}

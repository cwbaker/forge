//
// luaxx.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include <luaxx/luaxx.hpp>
#include <assert/assert.hpp>
#include <lua.hpp>
#include <algorithm>
#include <string.h>
#include <stdlib.h>

using std::max;
using namespace sweet::luaxx;

namespace sweet
{

namespace lua
{

extern void lua_dump_stack( lua_State* lua );

}

namespace luaxx
{

/** 
// The keyword used to store the address of objects. 
*/
const char* THIS_KEYWORD = "__luaxx_this";

/** 
// The keyword used to store the type of an object.
*/
const char* TYPE_KEYWORD = "__luaxx_type";

/** 
// The keyword used to store the weak objects table in the %Lua registry.
*/
const char* WEAK_OBJECTS_KEYWORD = "__luaxx_weak_objects";

/**
// Create a new, independent Lua state.
//
// @return 
//  The newly created lua_State.
*/
lua_State* luaxx_newstate()
{
    lua_State* lua_state = luaL_newstate();
    luaL_openlibs( lua_state );

    // Create the weak objects metatable and table.  The metatable is used to 
    // mark the table as storing weak references.  The table stores the tables 
    // that correspond to an application's C++ objects that have been weakened 
    // and have their lifetime managed by the Lua virtual machine.  The table
    // stored using the address of the C++ object as a key.
    //
    // The table uses weak keys and values so that tables may hold strong 
    // references back to C++ objects (via intrusive_ptrs, shared_ptrs, ptrs, 
    // etc) without the C++ objects creating cyclic references back to the 
    // Lua tables (because those C++ objects indirectly hold a reference to the 
    // table through their LuaObject).  The table can still be resolved from the
    // LuaObject because its address is stored in the weak objects table but this
    // reference isn't counted during a garbage collection sweep.
    lua_newtable( lua_state );
    lua_newtable( lua_state );
    lua_pushstring( lua_state, "kv" );
    lua_setfield( lua_state, -2, "__mode" );
    lua_setmetatable( lua_state, -2 );
    lua_setfield( lua_state, LUA_REGISTRYINDEX, WEAK_OBJECTS_KEYWORD );

    return lua_state;
}

/**
// Create a Lua object in \e lua identified by \e object.
//
// @param lua
//  The lua_State to create the object in.
//
// @param object
//  The address to use to identify the object.
//
// @param tname
//  The name of the type used to identify the attached object.
*/
void luaxx_create( lua_State* lua, void* object, const char* tname )
{
    SWEET_ASSERT( lua );
    SWEET_ASSERT( object );
    SWEET_ASSERT( tname );
    lua_newtable( lua );
    luaxx_attach( lua, object, tname );
    lua_pop( lua, 1 );
}

/**
// Destroy the Lua object in \e lua identified by \e object.
//
// Sets the value of the field stored under the THIS_POINTER key to nil.
// This stops the object being able to be used to refer back to an object
// in C++ after that C++ object has been destroyed even though the Lua 
// table will exist until it is garbage collected.
//
// Also removes references to the Lua table for the object from the Lua 
// registry and the weak references table so that the Lua table can't be
// reached via the C++ object address anymore.
//
// @param lua
//  The lua_State to destroy the object in.
//
// @param object
//  The address to use to identify the object.
*/
void luaxx_destroy( lua_State* lua, void* object )
{
    SWEET_ASSERT( lua );
    luaxx_push( lua, object );
    if ( lua_istable(lua, -1) )
    {
        lua_pushstring( lua, THIS_KEYWORD );
        lua_pushnil( lua );
        lua_rawset( lua, -3 );

        lua_pushstring( lua, TYPE_KEYWORD );
        lua_pushnil( lua );
        lua_rawset( lua, -3 );
    }   
    lua_pop( lua, 1 );
    luaxx_detach( lua, object );
}

/**
// Attach a C++ object to the Lua at the top of the stack.
//
// @param lua
//  The lua_State to create the object in.
//
// @param object
//  The address to use to identify the object.
//
// @param tname
//  The identifier that specifies the type of the object to attach to.
*/
void luaxx_attach( lua_State* lua, void* object, const char* tname )
{
    SWEET_ASSERT( lua );
    SWEET_ASSERT( lua_istable(lua, -1) );
    SWEET_ASSERT( object );
    SWEET_ASSERT( tname );

    // Set the this pointer stored in the Lua table to point to `object`.
    lua_pushstring( lua, THIS_KEYWORD );
    lua_pushlightuserdata( lua, object );
    lua_rawset( lua, -3 );

    // Set the type stored in the Lua table to the value of `tname`.
    lua_pushstring( lua, TYPE_KEYWORD );
    lua_pushstring( lua, tname );
    lua_rawset( lua, -3 );

    // Store the Lua table in the registry accessed by `object`.
    lua_pushlightuserdata( lua, object );
    lua_pushvalue( lua, -2 );
    lua_rawset( lua, LUA_REGISTRYINDEX );
}

/**
// Detach a C++ object from its associated Lua table.
//
// @param lua
//  The lua_State to remove references to Lua tables from \e object in 
//  (assumed not null).
//
// @param object
//  The address to remove the associated Lua tables of.
//
// @param weak
//  A pointer to a boolean that is set to true if the object was removed from
//  the weak objects table (null to ignore)
*/
void luaxx_detach( lua_State* lua, void* object, bool* weak )
{
    // Remove any reference to the object from the Lua registry.
    lua_pushlightuserdata( lua, object );
    lua_pushnil( lua );
    lua_rawset( lua, LUA_REGISTRYINDEX );

    // Remove any reference to the object from the weak objects table.
    lua_getfield( lua, LUA_REGISTRYINDEX, WEAK_OBJECTS_KEYWORD );
    if ( lua_istable(lua, -1) )
    {
        lua_pushlightuserdata( lua, object );
        lua_pushnil( lua );
        lua_rawset( lua, -3 );
        if ( weak )
        {
            *weak = true;
        }
    }
    lua_pop( lua, 1 );
}

/**
// Swap the Lua objects associated with \e object and \e other_object.
//
// Swaps the values referenced by the two addresses in the Lua registry so 
// that the Lua table that is associated with \e object is swapped with the
// Lua table that is associated with \e other_object and vice versa.
//
// The strong/weak relationship from the C++ address to the Lua table is 
// *not* swapped.  For example if \e object has a strong relationship from 
// C++ to the first Lua table and \e other_object has a weak relationship 
// from C++ to the second Lua table then, after swapping, \e object has a 
// strong relationship from C++ to the second Lua table and \e other_object 
// has a weak relationship from C++ to the first Lua table.  The values are 
// swapped but the strength/weakness of the relationship from \e object and
// \e other_object to their Lua tables remains unchanged.
//
// @param lua
//  The Lua state to swap the objects in.
//
// @param object
//  The address used to associate with the first Lua object.
//
// @param other_object
//  The address used to associate with the second Lua object.
*/
void luaxx_swap( lua_State* lua, void* object, void* other_object )
{
    // Push the Lua table associated with the first object onto the stack and 
    // remove it from the Lua registry or weak objects table.
    lua_pushlightuserdata( lua, other_object );
    luaxx_push( lua, object );
    bool object_weak = false;
    luaxx_detach( lua, object, &object_weak );

    // Push the Lua table associated with the second object onto the stack and
    // remove it from the Lua registry or weak objects table.
    lua_pushlightuserdata( lua, object );
    luaxx_push( lua, other_object );
    bool other_object_weak = false;
    luaxx_detach( lua, other_object, &other_object_weak );

    // Swap associations between `object` and `other_object` and their entries
    // in the Lua registry.
    lua_rawset( lua, LUA_REGISTRYINDEX );
    lua_rawset( lua, LUA_REGISTRYINDEX );

    // Restore weak relationships from the first and second objects to their
    // associated Lua tables.  The strength of the relationship remains as it
    // was before this function was called; i.e. the values are swapped but 
    // the strengths of the relationships from C++ to those values are not.   
    if ( object_weak )
    {
        luaxx_weaken( lua, object );
    }

    if ( other_object_weak )
    {
        luaxx_weaken( lua, other_object );
    }
}

/**
// Weaken the object in \e lua identified by \e object.
//
// This moves the table associated with \e object from the Lua registry into
// the weak objects table.  The weak objects table stores only weak references
// to its contents.  This means that the table associated with \e object will
// be able to be garbage collected once there are no more references to it
// from Lua.
//
// @param lua
//  The lua_State to weaken the object in.
//
// @param object
//  The address to use to identify the object.
*/
void luaxx_weaken( lua_State* lua, void* object )
{
    SWEET_ASSERT( lua );
    if ( object )
    {
        // Get the weak objects table from the Lua registry.
        lua_getfield( lua, LUA_REGISTRYINDEX, WEAK_OBJECTS_KEYWORD );
        SWEET_ASSERT( lua_istable(lua, -1) );

        // If there is a table for the object in the Lua registry then move that
        // table from the registry to the weak objects table otherwise assume that
        // the object is already weakened and its table already exists in the
        // weak objects table and quietly do nothing.
        lua_pushlightuserdata( lua, object );
        lua_rawget( lua, LUA_REGISTRYINDEX );
        if ( lua_istable(lua, -1) )
        {
            // Add the object's table to the weak objects table.
            lua_pushlightuserdata( lua, object );
            lua_pushvalue( lua, -2 );
            lua_rawset( lua, -4 );

            // Remove the object's table from the Lua registry.
            lua_pushlightuserdata( lua, object );
            lua_pushnil( lua );
            lua_rawset( lua, LUA_REGISTRYINDEX );
        }
        lua_pop( lua, 2 );
    }
}

/**
// Strengthen the object in \e lua identified by \e object.
//
// This moves the table associated with \e object from the weak objects
// table back to the Lua registry.
//
// @param lua
//  The lua_State to strengthen the object in.
//
// @param object
//  The address to use to identify the object.
*/
void luaxx_strengthen( lua_State* lua, void* object )
{
    SWEET_ASSERT( lua );
    if ( object )
    {
        // Get the weak objects table from the Lua registry.
        lua_getfield( lua, LUA_REGISTRYINDEX, WEAK_OBJECTS_KEYWORD );
        SWEET_ASSERT( lua_istable(lua, -1) );

        // If there is a table for the object in the weak objects table then 
        // move that table from the weak objects table to the registry 
        // otherwise assume that the object is already weakened and its table
        // already exists in the weak objects table and quietly do nothing.
        lua_pushlightuserdata( lua, object );
        lua_rawget( lua, -2 );
        if ( lua_istable(lua, -1) )
        {
            // Add the object's table to the Lua registry.
            lua_pushlightuserdata( lua, object );
            lua_pushvalue( lua, -2 );
            lua_rawset( lua, LUA_REGISTRYINDEX );

            // Remove the object's table from the weak objects table.
            lua_pushlightuserdata( lua, object );
            lua_pushnil( lua );
            lua_rawset( lua, -4 );
        }
        lua_pop( lua, 2 );
    }
}

/**
// Push \e object's equivalent table onto the stack in \e lua.
//
// @param lua
//  The lua_State to push the object onto the stack of.
//
// @param object
//  The address that identifies the object previously passed to 
//  `luaxx_create()` (can be null).
//
// @return
//  True if there was a table corresponding to \e object in \e lua otherwise
//  false.
*/
bool luaxx_push( lua_State* lua, void* object )
{
    SWEET_ASSERT( lua );
    
    if ( object )
    {
        lua_pushlightuserdata( lua, object );
        lua_rawget( lua, LUA_REGISTRYINDEX );
        SWEET_ASSERT( lua_istable(lua, -1) || lua_isnil(lua, -1) );
        if ( lua_isnil(lua, -1) )
        {
            lua_pop( lua, 1 );
            lua_getfield( lua, LUA_REGISTRYINDEX, WEAK_OBJECTS_KEYWORD );
            SWEET_ASSERT( lua_istable(lua, -1) );
            lua_pushlightuserdata( lua, object );
            lua_rawget( lua, -2 );
            lua_remove( lua, -2 );
        }

        // If anything other than a table ends up on the top of the stack
        // after looking for tables in the Lua registry and the weak 
        // objects table then pop that and push nil in its place to allow 
        // later error handling to report a problem.  This usually means 
        // that no table has been created for the C++ object via 
        // `luaxx_create()` or `luaxx_attach()`.
        if ( !lua_istable(lua, -1) && !lua_isnil(lua, -1) )
        {
            lua_pop( lua, 1 );
            lua_pushnil( lua );
        }         
    }
    else
    {
        lua_pushnil( lua );
    }
    return lua_istable( lua, -1 );
}

/**
// Get the address of the object at \e position in \e lua's stack.
//
// @param lua
//  The lua_State to push the object onto the stack of.
//
// @param position
//  The absolute position in the stack to get the object from (it is assumed
//  that this position is an absolute position; that is \e position > 0 or
//  position < LUA_REGISTRYINDEX (-10000)).
//
// @param tname
//  The type to check that the object is (set luaL_newmetatable()).
//
// @return
//  The address of the object or null if there is no value at that position
//  on the stack or if the value at that position is nil.
*/
void* luaxx_to( lua_State* lua, int position, const char* tname )
{
    SWEET_ASSERT( lua );
    SWEET_ASSERT( position > 0 || position < LUA_REGISTRYINDEX );

    void* object = nullptr;
    if ( !lua_isnoneornil(lua, position) )
    {
        lua_pushstring( lua, tname );
        lua_pushstring( lua, TYPE_KEYWORD );
        lua_gettable( lua, position );
        if ( lua_rawequal(lua, -1, -2) )
        {
            lua_pushstring( lua, THIS_KEYWORD );
            lua_gettable( lua, position );
            if ( lua_islightuserdata(lua, -1) )
            {
                object = lua_touserdata( lua, -1 );
            }
            lua_pop( lua, 1 );
        }
        lua_pop( lua, 2 );
    }
    return object;
}

/**
// Get the address of the object at \e position in \e lua's stack.
//
// If the object at \e position in the stack can't be converted to a C++ 
// object pointer because it is nil, isn't a table, doesn't have a matching
// type, or has no this pointer then an error is generated.
//
// @param lua
//  The lua_State to push the object onto the stack of.
//
// @param position
//  The absolute position in the stack to get the object from (it is assumed
//  that this position is an absolute position; that is \e position > 0 or
//  position < LUA_REGISTRYINDEX (-10000)).
//
// @param tname
//  The type to check that the object is (set luaL_newmetatable()).
//
// @return
//  The address of the object.
*/
void* luaxx_check( lua_State* lua, int position, const char* tname )
{
    void* object = luaxx_to( lua, position, tname );
    luaL_argcheck( lua, object != nullptr, position, "this pointer not set or C++ object has been destroyed" );
    return object;
}

/**
// Implement the lua_Alloc function using `realloc()` and `free().
//
// @param context
//  Application supplied context (ignored).
// 
// @param ptr
//  The address of any existing allocation to be reallocated or null to 
//  allocate a new block of memory.
// 
// @param osize
//  The old size of the memory block (ignored).
// 
// @param nsize
//  The size of the memory block to allocate.
// 
// @return
//  A pointer to the newly allocated memory.
*/
void* luaxx_allocate( void* /*context*/, void* ptr, size_t /*osize*/, size_t nsize )
{
    if ( nsize == 0 ) 
    {
        free( ptr );
        return 0;
    }
    else
    {
        return realloc( ptr, nsize );
    }
}

/**
// Do a Lua stack trace.
//
// This function is pushed onto the stack for each Lua and LuaThread object 
// that is created.  Its position is passed to lua_pcall() when this 
// AddParameter object makes its call to Lua.  If an error occurs during the 
// execution of the call this function is called with the string describing 
// the error at the top of the stack.
//
// This function then walks back up the Lua stack recording each function that
// has been called from the error occuring back up to the first call into
// the Lua virtual machine from C++.
//
// It then returns all of this information concatenated into a single string 
// that is passed back to the application as the value at the top of the Lua
// stack.
//
// @param lua_state
//  The lua_State to do a stack trace for.
//
// @return
//  Always returns 1.
*/
int luaxx_stack_trace_for_call( lua_State* lua_state )
{
    const int STACK_TRACE_BEGIN = 0;
    const int STACK_TRACE_END = 6;

    lua_Debug debug;
    memset( &debug, 0, sizeof(debug) );
    
    lua_pushliteral( lua_state, ".\nstack trace:" );

    int level = STACK_TRACE_BEGIN;
    while ( level < STACK_TRACE_END && lua_getstack(lua_state, level, &debug) )
    {
        lua_getinfo( lua_state, "Snl", &debug );
    
        // Source and line number.
        lua_pushliteral( lua_state, "\n  " );        
        if ( debug.currentline > 0 )
        {
            lua_pushfstring( lua_state, "%s(%d) : ", debug.source, debug.currentline );
        }
        else
        {
            lua_pushfstring( lua_state, "%s(1) : ", debug.source );
        }

        // Application provided name or other implicit location information.
        if ( *debug.namewhat != '\0' )
        {
            lua_pushfstring( lua_state, "in function '%s'", debug.name );
        }
        else 
        {
            switch ( *debug.what )
            {
                case 'm':
                    lua_pushfstring( lua_state, "main");
                    break;

                default:
                    lua_pushfstring( lua_state, "in function <%s(%d)>", debug.source, debug.linedefined );
            }
        }

        lua_concat( lua_state, lua_gettop(lua_state) );
        ++level;
    }

    lua_concat( lua_state, lua_gettop(lua_state) );
    return 1;
}

/**
// Do a Lua stack trace after lua_resume() has been called to call a function
// and that function has failed.
//
// This function walks back up the Lua stack recording each function that had 
// been called from the error occuring back up to the first call into
// Lua from C++.
//
// @param lua_state
//  The lua_State to do a stack trace for.
//
// @param stack_trace_enabled
//  True to do a stack trace or false to just retrieve the error message from
//  the top of the stack.
//
// @param message
//  The buffer to put the stack trace message into.
//
// @param length
//  The maximum length of the stack trace message (in bytes).
//
// @return
//  Returns the string passed in \e message for convenience.
*/
const char* luaxx_stack_trace_for_resume( lua_State* lua_state, bool stack_trace_enabled, char* message, int length )
{
    SWEET_ASSERT( lua_state != NULL );
    SWEET_ASSERT( message != NULL );
    SWEET_ASSERT( length > 0 );

    int written = 0;
    memset( message, 0, length );
    written += snprintf( message + written, max(length - written, 0), "%s", lua_isstring(lua_state, -1) ? lua_tostring(lua_state, -1) : "Unknown error" );

    if ( stack_trace_enabled )
    {
        static const int STACK_TRACE_BEGIN = 0;
        static const int STACK_TRACE_END   = 6;

        written += snprintf( message + written, max(length - written, 0), ".\nstack trace:" );

        lua_Debug debug;
        memset( &debug, 0, sizeof(debug) );

        int level = STACK_TRACE_BEGIN;
        while ( level < STACK_TRACE_END && length - written > 0 && lua_getstack(lua_state, level, &debug) )
        {
            lua_getinfo( lua_state, "Snl", &debug );
        
            // Source and line number.
            written += snprintf( message + written, max(length - written, 0), "\n  " );        
            if ( debug.currentline > 0 )
            {
                written += snprintf( message + written, max(length - written, 0), "%s(%d) : ", debug.source, debug.currentline );
            }
            else
            {
                written += snprintf( message + written, max(length - written, 0), "%s(1) : ", debug.source );
            }

            // Application provided name or other implicit location information.
            if ( *debug.namewhat != '\0' )
            {
                written += snprintf( message + written, max(length - written, 0), "in function '%s'", debug.name );
            }
            else 
            {
                switch ( *debug.what )
                {
                    case 'm':
                        written += snprintf( message + written, max(length - written, 0), "main");
                        break;

                    default:
                        written += snprintf( message + written, max(length - written, 0), "in function <%s(%d)>", debug.source, debug.linedefined );
                        break;
                }
            }

            ++level;
        }
    }

    message [length - 1] = 0;
    return message;
}

}

}

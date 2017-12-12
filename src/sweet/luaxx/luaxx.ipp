#ifndef SWEET_LUAXX_IPP_INCLUDED
#define SWEET_LUAXX_IPP_INCLUDED

#include "luaxx.hpp"
#include <sweet/assert/assert.hpp>
#include <lua/lua.hpp>

namespace sweet
{

namespace luaxx
{

/**
// Garbage collect the user data allocated for an arbitrary C++ object.
//
// This calls the in place destructor for the object.
//
// @param lua
//  The lua_State that is garbage collecting the object.
// 
// @return
//  Always returns 0.
*/
template <class Type> 
int luaxx_gc( lua_State* lua )
{
    SWEET_ASSERT( lua );
    SWEET_ASSERT( lua_isuserdata(lua, 1) );
    Type* object = static_cast<Type*>( lua_touserdata(lua, 1) );
    SWEET_ASSERT( object );
    object->Type::~Type();
    lua_pop( lua, 1 );
    return 0;
}

/**
// Push a C++ object onto the stack by value.
//
// Copies \e value into a new object allocated as Lua userdata and sets the
// class's destructor to be called when the userdata value is garbage 
// collected (note that this may be some time after the userdata value is 
// no longer referenced).
//
// @param lua
//  The lua_State to push the object onto the stack of.
//
// @param value
//  The object to push by value.
//
// @param tname
//  The name of the metatable to use for the value (assumed to contain an
//  appropriate `__gc` metamethod, see `luaL_newmetatable()` and 
//  `luaxx_gc<>()`)
*/
template <class Type> 
void luaxx_newuserdata( lua_State* lua, const Type& value, const char* tname )
{
    SWEET_ASSERT( lua );
    void* copied_value = lua_newuserdata( lua, sizeof(Type) );
    luaL_setmetatable( lua, tname );
    SWEET_ASSERT( copied_value );
    new (copied_value) Type( value );
}

/**
// Convert a C++ standard library sequence defined by a pair of iterators 
// into a Lua iterator.
//
// This is intended to be used as a closure that has the pair of iterators
// stored in user data bound as up values.
//
// @param lua
//  The lua_State.
// 
// @return
//  Always returns 1.
*/
template <class Iterator>
int luaxx_iterator( lua_State* lua )
{
    SWEET_ASSERT( lua );

    // Get the iterator for the current and end positions.
    Iterator* iterator = (Iterator*) lua_touserdata( lua, lua_upvalueindex(1) );
    Iterator* end = (Iterator*) lua_touserdata( lua, lua_upvalueindex(2) );
    SWEET_ASSERT( iterator );
    SWEET_ASSERT( end );

    // If the current iterator is not at the end then push the current value and
    // iterate to the next position.  Otherwise push nil to indicate that the end
    // of the sequence has been reached.
    if ( *iterator != *end )
    {
        lua_pushvalue( lua, lua_upvalueindex(1) );
        ++(*iterator);
    }
    else
    {
        lua_pushnil( lua );
    }

    return 1;
}

/**
// Create a Lua iterator to iterate over the range [\e start, \e finish).
//
// Copies of the iterators \e start and \e finish are made by calling the in 
// place copy constructor on memory allocated as Lua userdata.  A garbage 
// collection method is set for the metatable for the userdata so that the 
// iterators are destroyed correctly when the userdata is garbage collected.
//
// The userdata values containing the copied iterators are then bound as 
// upvalues of an iterator function (see lua_iterator()) that iterates over 
// the range returning nil when the end is reached.
//
// @param lua_state
//  The lua_State.
// 
// @param start
//  The iterator at the position to being iterating over.
//
// @param finish
//  The iterator one position past the finish element to iterate over.
*/
template <class Iterator>
void luaxx_pushiterator( lua_State* lua_state, Iterator start, Iterator finish, const char* tname )
{
    SWEET_ASSERT( lua_state );
    luaxx_newuserdata<Iterator>( lua_state, start, tname );
    luaxx_newuserdata<Iterator>( lua_state, finish, tname );
    lua_pushcclosure( lua_state, &luaxx_iterator<Iterator>, 2 );
}

}

}

#endif

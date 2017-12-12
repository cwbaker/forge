#ifndef SWEET_LUAXX_HPP_INCLUDED
#define SWEET_LUAXX_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace luaxx
{

extern const char* THIS_KEYWORD;
extern const char* TYPE_KEYWORD;
extern const char* WEAK_OBJECTS_KEYWORD;

void luaxx_create( lua_State* lua, void* object, const char* tname );
void luaxx_destroy( lua_State* lua, void* object );
void luaxx_attach( lua_State* lua, void* object, const char* tname );
void luaxx_detach( lua_State* lua, void* object, bool* weak = nullptr );
void luaxx_swap( lua_State* lua, void* object, void* other_object );
void luaxx_weaken( lua_State* lua, void* object );
void luaxx_strengthen( lua_State* lua, void* object );
bool luaxx_push( lua_State* lua, void* object );
void* luaxx_to( lua_State* lua, int position, const char* tname );
void* luaxx_check( lua_State* l, int position, const char* tname );

template <class Type> int luaxx_gc( lua_State* lua );
template <class Type> void luaxx_newuserdata( lua_State* lua, const Type& value, const char* tname );
template <class Iterator> int luaxx_iterator( lua_State* lua );
template <class Iterator> void luaxx_pushiterator( lua_State* lua, Iterator start, Iterator finish );

}

}

#include "luaxx.ipp"

#endif

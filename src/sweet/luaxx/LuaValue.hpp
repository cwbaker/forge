#ifndef SWEET_LUAXX_LUAVALUE_HPP_INCLUDED
#define SWEET_LUAXX_LUAVALUE_HPP_INCLUDED

struct lua_State;

namespace sweet
{

namespace luaxx
{

/**
// Hold a reference to a value in Lua so that it doesn't get garbage 
// collected.
//
// Stores a reference to the Lua value in the Lua registry using the address
// of the LuaValue as a key so that Lua considers the value referenced and 
// therefore not available for garbage collection.
*/
class LuaValue
{
    lua_State* lua_state_;
    
public:
    LuaValue();
    LuaValue( lua_State* lua_state, int position );
    LuaValue( lua_State* lua_state, lua_State* coroutine, int position );
    LuaValue( const LuaValue& value );
    LuaValue& operator=( const LuaValue& value );
    ~LuaValue();
};

}

}

#endif
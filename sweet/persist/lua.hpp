//
// lua.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_LUA_HPP_INCLUDED
#define SWEET_PERSIST_LUA_HPP_INCLUDED

#include <sweet/lua/lua_/lua.h>

namespace sweet
{

namespace persist
{

template <class Archive> 
void enter( Archive& archive, lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    lua_pushlightuserdata( lua_state, lua_state );
    lua_newtable( lua_state );
    lua_rawset( lua_state, LUA_REGISTRYINDEX );
}

template <class Archive> 
void exit( Archive& archive, lua_State* lua_state )
{
    SWEET_ASSERT( lua_state );
    lua_pushlightuserdata( lua_state, lua_state );
    lua_pushnil( lua_state );
    lua_rawset( lua_state, LUA_REGISTRYINDEX );
}

template <class Archive>
void save_lua_table( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
{
    SWEET_ASSERT( archive.is_writing() );
    SWEET_ASSERT( name );
    SWEET_ASSERT( lua_state );

    int table_index = lua_gettop( lua_state );
    SWEET_ASSERT( lua_istable(lua_state, table_index) || lua_isnil(lua_state, table_index) );

    const void* address = lua_istable( lua_state, table_index ) ? lua_topointer( lua_state, table_index ) : NULL;

//
// An ObjectGuard<> is created here even if there isn't a table to persist
// so that binary Archives have a symmetric load process for loading
// their metatables.  If an empty element isn't saved in a binary Archive
// for a nil metatable then the binary load recurses infinitely because there
// is always another object to load that is misinterpreted as the metatable.
//
    ObjectGuard<Archive> guard( archive, name, address, MODE_VALUE, lua_istable(lua_state, table_index) ? 1 : 0 );
    if ( lua_istable(lua_state, table_index) && !archive.is_tracked(address) )
    {
        archive.track( address );
        lua_checkstack( lua_state, 8 );

    //
    // Persist the metatable.  A nil value is persisted even if the metatable
    // doesn't exist so that the load process is symmetric.
    //
        if ( metatable )
        {
            if ( !lua_getmetatable(lua_state, table_index) )
            {
                lua_pushnil( lua_state );
            }
            save_lua_table( archive, "metatable", lua_state, true );
            lua_pop( lua_state, 1 );
        }

    //
    // Count the number of boolean, number, string, and table fields in the 
    // table that are stored with strings for keys.  These are the fields 
    // that will be persisted.
    //
        int fields = 0;
        lua_pushnil( lua_state );
        while ( lua_next(lua_state, table_index) != 0 ) 
        {
            int key_type = lua_type( lua_state, -2 );

            if ( key_type == LUA_TSTRING || key_type == LUA_TNUMBER )
            {
                switch ( lua_type(lua_state, -1) )
                {
                    case LUA_TBOOLEAN:
                    case LUA_TNUMBER:
                    case LUA_TSTRING:
                    case LUA_TTABLE:
                        ++fields;
                        break;

                    default:
                        break;
                }
            }

            lua_pop( lua_state, 1 );
        }

    //
    // Persist the fields in the table.
    //
        ObjectGuard<Archive> attributes_guard( archive, "attributes", NULL, MODE_VALUE, fields );
        if ( fields > 0 )
        {
            lua_pushnil( lua_state );
            while ( lua_next(lua_state, table_index) != 0 ) 
            {
                int key_type   = lua_type( lua_state, -2 );
                int value_type = lua_type( lua_state, -1 );

                if ( (key_type == LUA_TSTRING || key_type == LUA_TNUMBER) && (value_type == LUA_TBOOLEAN || value_type == LUA_TNUMBER || value_type == LUA_TSTRING || value_type == LUA_TTABLE) )
                {
                    ObjectGuard<Archive> attribute_guard( archive, "attribute", NULL, MODE_VALUE, 1 );

                    if ( key_type == LUA_TSTRING )
                    {
                        std::string key = lua_tostring( lua_state, -2 );
                        archive.value( "key", key );
                    }
                    else
                    {
                        lua_pushvalue( lua_state, -2 );
                        std::string key = lua_tostring( lua_state, -1 );
                        archive.value( "key", key );
                        lua_pop( lua_state, 1 );
                    }

                    switch ( value_type )
                    {
                        case LUA_TBOOLEAN:
                        {
                            BasicType type = TYPE_BOOLEAN;
                            bool value = lua_toboolean( lua_state, -1 ) ? true : false;
                            archive.value( "type", type, enum_filter(BASIC_TYPES) );           
                            archive.value( "value", value );
                            break;
                        }

                        case LUA_TNUMBER:
                        {
                            BasicType type = TYPE_NUMBER;
                            lua_Number value = lua_tonumber( lua_state, -1 );
                            archive.value( "type", type, enum_filter(BASIC_TYPES) );           
                            archive.value( "value", value );
                            break;
                        }

                        case LUA_TSTRING:
                        {
                            BasicType type = TYPE_STRING;
                            std::string value = lua_tostring( lua_state, -1 );
                            archive.value( "type", type, enum_filter(BASIC_TYPES) );           
                            archive.value( "value", value );
                            break;
                        }

                        case LUA_TTABLE:
                        {
                            BasicType type = TYPE_OBJECT;
                            archive.value( "type", type, enum_filter(BASIC_TYPES) );
                            save_lua_table( archive, "table", lua_state, true );
                            break;
                        }

                        default:
                            SWEET_ASSERT( false );
                            break;
                    }
                }

                lua_pop( lua_state, 1 );
            }
        }
    }
}

template <class Archive>
void load_lua_table( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
{
    SWEET_ASSERT( archive.is_reading() );
    SWEET_ASSERT( name );
    SWEET_ASSERT( lua_state );

    ObjectGuard<Archive> guard( archive, name, NULL, MODE_VALUE, 1 );
    if ( archive.is_object() )
    {
        lua_checkstack( lua_state, 8 );
        lua_pushlightuserdata( lua_state, lua_state );
        lua_rawget( lua_state, LUA_REGISTRYINDEX );
        const int tracking_table = lua_gettop( lua_state );
        SWEET_ASSERT( lua_istable(lua_state, tracking_table) );

        const void* address = archive.get_address();
        lua_pushlightuserdata( lua_state, const_cast<void*>(address) );
        lua_rawget( lua_state, tracking_table );
        if ( lua_isnil(lua_state, -1) )
        {
        //
        // Create the table and store it under its address in the tracking
        // table that is stored under the address of the lua_State in the
        // Lua registry.
        //
            lua_pop( lua_state, 1 );
            lua_pushlightuserdata( lua_state, const_cast<void*>(address) );
            lua_newtable( lua_state );
            lua_rawset( lua_state, tracking_table );
            lua_pushlightuserdata( lua_state, const_cast<void*>(address) );
            lua_rawget( lua_state, tracking_table );
            int table = lua_gettop( lua_state );
            SWEET_ASSERT( lua_istable(lua_state, table) );

        //
        // Load the metatable if it is present.
        //
            if ( metatable )
            {
                load_lua_table( archive, "metatable", lua_state, true );
                SWEET_ASSERT( lua_istable(lua_state, -1) || lua_isnil(lua_state, -1) );
                lua_setmetatable( lua_state, table );
            }

        //
        // Load the fields of the table from the Archive.
        //
            ObjectGuard<Archive> attributes_guard( archive, "attributes", NULL, MODE_VALUE );
            if ( archive.is_object() )
            {
                while ( archive.find_next_object("attribute") )
                {
                    ObjectGuard<Archive> attribute_guard( archive, "attribute", NULL, MODE_VALUE );
                    SWEET_ASSERT( archive.is_object() );

                    std::string key;
                    archive.value( "key", key );

                    BasicType type = TYPE_VOID;
                    archive.value( "type", type, enum_filter(BASIC_TYPES) );

                    int index = ::atoi( key.c_str() );
                    if ( index == 0 )
                    {
                        lua_pushlstring( lua_state, key.c_str(), key.length() );
                    }
                    else
                    {
                        lua_pushinteger( lua_state, ::atoi(key.c_str()) );
                    }

                    switch ( type )
                    {
                        case TYPE_BOOLEAN:
                        {
                            bool value = false;
                            archive.value( "value", value );
                            lua_pushboolean( lua_state, value ? 1 : 0 );
                            lua_rawset( lua_state, table );
                            break;
                        }

                        case TYPE_NUMBER:
                        {
                            lua_Number value = 0;
                            archive.value( "value", value );
                            lua_pushnumber( lua_state, value );
                            lua_rawset( lua_state, table );
                            break;
                        }

                        case TYPE_STRING:
                        {
                            std::string value;
                            archive.value( "value", value );
                            lua_pushlstring( lua_state, value.c_str(), value.length() );
                            lua_rawset( lua_state, table );
                            break;
                        }

                        case TYPE_OBJECT:
                        {
                            load_lua_table( archive, "table", lua_state, true );
                            lua_rawset( lua_state, table );
                            break;
                        }

                    //
                    // For unsupported types (or elements that don't specify a 
                    // type) make sure that the string or integer key pushed
                    // previously is popped.
                    //
                        default:
                            lua_pop( lua_state, 1 );
                            break;
                    }
                }
            }
        }

        lua_remove( lua_state, tracking_table );
    }
    else
    {
        lua_pushnil( lua_state );
    }
}

template <class Archive>
void resolve_lua_table( Archive& archive, lua_State* lua_state, bool metatable )
{
}

template <class Archive, class Type>
struct PersistLuaTable
{
    static void persist( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
    {
        SWEET_ASSERT( false );
    }
};

template <class Archive>
struct PersistLuaTable<Archive, ArchiveTypeWriter>
{
    static void persist( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
    {
        save_lua_table( archive, name, lua_state, metatable );
    }
};

template <class Archive>
struct PersistLuaTable<Archive, ArchiveTypeReader>
{
    static void persist( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
    {
        load_lua_table( archive, name, lua_state, metatable );
    }
};

template <class Archive>
struct PersistLuaTable<Archive, ArchiveTypeResolver>
{
    static void persist( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
    {
        resolve_lua_table( archive, lua_state, metatable );
    }
};

template <class Archive> 
void persist_lua_table( Archive& archive, const char* name, lua_State* lua_state, bool metatable )
{
    PersistLuaTable<Archive, typename Archive::archive_type>::persist( archive, name, lua_state, metatable );
}

}

}

#endif

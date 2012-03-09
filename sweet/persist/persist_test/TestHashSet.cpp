//
// TestHashSet.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/hash_set.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestHashSet )
{
    struct Model
    {
        stdext::hash_set<Item> items_;

        Model()
        {
            Item item;
            item.m_string = "item_000";
            items_.insert( item );
            item.m_string = "item_001";
            items_.insert( item );
            item.m_string = "item_002";
            items_.insert( item );
            item.m_string = "item_003";
            items_.insert( item );
        }

        void clear()
        {
            items_.clear();
        }

        template <class Archive> void enter( Archive& archive )
        {
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.value( "items", "item", items_ );
        }
    };

    TEST( TestHashSetXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestHashSet.xml" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashSetJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestHashSet.json" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashSetLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestHashSet.lua" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashSetBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestHashSet.dat" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
}

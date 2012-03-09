//
// TestHashMultiset.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/hash_set.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestHashMultiset )
{
    struct Model
    {
        stdext::hash_multiset<Item> items_;

        Model()
        {
            Item item;
            item.m_string = "item_000";
            items_.insert( item );
            item.m_string = "item_001";
            items_.insert( item );
            item.m_string = "item_001";
            items_.insert( item );
            item.m_string = "item_002";
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

    TEST( TestHashMultisetXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestHashMultiset.xml" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashMultisetJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestHashMultiset.json" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashMultisetLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestHashMultiset.lua" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashMultisetBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestHashMultiset.dat" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
}

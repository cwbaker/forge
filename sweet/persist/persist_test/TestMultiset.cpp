//
// TestMultiset.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/set.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestMultiset )
{
    struct Model
    {
        std::multiset<Item> items_;

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

    TEST( TestMultisetXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestMultiset.xml" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestMultisetJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestMultiset.json" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestMultisetLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestMultiset.lua" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestMultisetBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestMultiset.dat" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
}

//
// TestList.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/list.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestList )
{
    struct Model
    {
        std::list<Item> items_;

        Model()
        {
            Item item;
            item.m_string = "item_000";
            items_.push_back( item );
            item.m_string = "item_001";
            items_.push_back( item );
            item.m_string = "item_002";
            items_.push_back( item );
            item.m_string = "item_003";
            items_.push_back( item );
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

    TEST( TestListXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestList.xml" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestListJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestList.json" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestListLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestList.lua" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestListBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestList.dat" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
}

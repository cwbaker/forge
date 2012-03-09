//
// TestMultimap.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/map.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestMultimap )
{
    struct Model
    {
        std::multimap<std::string, Item> items_;

        Model()
        {
            using std::make_pair;

            Item item;
            item.m_string = "item_000";
            items_.insert( make_pair(item.m_string, item) );
            item.m_string = "item_001";
            items_.insert( make_pair(item.m_string, item) );
            item.m_string = "item_002";
            items_.insert( make_pair(item.m_string, item) );
            item.m_string = "item_003";
            items_.insert( make_pair(item.m_string, item) );
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

    TEST( TestMultimapXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestMultimap.xml" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestMultimapJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestMultimap.json" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestMultimapLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestMultimap.lua" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestMultimapBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestMultimap.dat" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
}

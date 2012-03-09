//
// TestHashMultimap.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/hash_map.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestHashMultimap )
{
    struct Model
    {
        stdext::hash_multimap<std::string, Item> items_;

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

    TEST( TestHashMultimapXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestHashMultimap.xml" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashMultimapJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestHashMultimap.json" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashMultimapLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestHashMultimap.lua" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
    
    TEST( TestHashMultimapBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestHashMultimap.dat" );
        CHECK( body.written_model_.items_ == body.read_model_.items_ );
    }
}

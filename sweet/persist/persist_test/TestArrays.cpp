//
// TestArrays.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestArrays )
{
    struct Model
    {
        static const unsigned int ITEM_COUNT = 4;

        Item  items_ [ITEM_COUNT];
        Item  multi_items_ [ITEM_COUNT][ITEM_COUNT];
        Item* raw_items_ [ITEM_COUNT];

        Model()
          //items_,
          //multi_items_,
          //raw_items_,
        {
            items_[0] = Item( "000", 2 );
            items_[1] = Item( "001", 2 );
            items_[2] = Item( "002", 2 );
            items_[3] = Item( "003", 2 );

            multi_items_[0][0] = Item( "00.00", 2 );
            multi_items_[0][1] = Item( "00.01", 3 );
            multi_items_[0][2] = Item( "00.02", 4 );
            multi_items_[0][3] = Item( "00.03", 5 );
            multi_items_[1][0] = Item( "01.00", 1 );
            multi_items_[1][1] = Item( "01.01", 6 );
            multi_items_[1][2] = Item( "01.02", 8 );
            multi_items_[1][3] = Item( "01.03", 9 );
            multi_items_[2][0] = Item( "02.00", 5 );
            multi_items_[2][1] = Item( "02.01", 2 );
            multi_items_[2][2] = Item( "02.02", 7 );
            multi_items_[2][3] = Item( "02.03", 4 );
            multi_items_[3][0] = Item( "03.00", 3 );
            multi_items_[3][1] = Item( "03.01", 7 );
            multi_items_[3][2] = Item( "03.02", 8 );
            multi_items_[3][3] = Item( "03.03", 1 );

            raw_items_[0] = &items_[2];
            raw_items_[1] = &items_[2];
            raw_items_[2] = &items_[1];
            raw_items_[3] = &items_[3];
        }

        void clear()
        {
            for ( int i = 0; i < ITEM_COUNT; ++i )
            {
                items_[i] = Item();
                raw_items_[i] = NULL;

                for ( int j = 0; j < ITEM_COUNT; ++j )
                {
                    multi_items_[i][j] = Item();
                }
            }
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.template declare <Item> ( "Item", PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", 1, *this );            
            archive.value( "items", "item", items_ );
            archive.value( "multi_items", "item", multi_items_ );
            archive.refer( "raw_items", "item", raw_items_ );
        }

        void check( const Model& model )
        {
            int i = 0;
            while ( i < ITEM_COUNT && items_[i] == model.items_[i] )
            {
                ++i;
            }
            CHECK( i == ITEM_COUNT );

            i = 0;
            while ( i < ITEM_COUNT && *raw_items_[i] == *model.raw_items_[i] )
            {
                ++i;
            }
            CHECK( i == ITEM_COUNT );
        }
    };

    TEST( TestArraysXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestArrays.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestArraysJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestArrays.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestArraysLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestArrays.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestArraysBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestArrays.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

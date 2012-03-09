//
// TestContainersInContainers.cpp
// Copyright (c) 2008 - 2010 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"

#if defined SWEET_BOOST_ENABLED

#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <sweet/persist/list.hpp>
#include <sweet/persist/set.hpp>
#include <sweet/persist/shared_ptr.hpp>
#include <sweet/persist/weak_ptr.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace boost;
using namespace sweet::persist;

SUITE( TestContainersInContainers )
{
    struct Model
    {
        std::list<std::list<weak_ptr<Item>>>    pre_references_;
        std::list<std::vector<Item>>            list_of_vectors_;
        std::vector<std::set<shared_ptr<Item>>> vector_of_shared_ptr_sets_;
        std::list<std::list<weak_ptr<Item>>>    post_references_;

        Model()
        {
            using boost::shared_ptr;
            using boost::weak_ptr;

            std::vector<Item> items;
            items.push_back( Item("item_001", 1) );
            items.push_back( Item("item_002", 2) );
            items.push_back( Item("item_003", 3) );
            items.push_back( Item("item_004", 4) );
            list_of_vectors_.push_back( items );

            items.clear();
            items.push_back( Item("item_005", 1) );
            items.push_back( Item("item_006", 2) );
            list_of_vectors_.push_back( items );

            items.clear();
            items.push_back( Item("item_007", 3) );
            items.push_back( Item("item_008", 4) );
            list_of_vectors_.push_back( items );

            shared_ptr<Item> shared_item_001( new Item("shared_item_001", 1) );
            shared_ptr<Item> shared_item_002( new Item("shared_item_002", 2) );
            shared_ptr<Item> shared_item_003( new Item("shared_item_003", 3) );
            shared_ptr<Item> shared_item_004( new Item("shared_item_004", 4) );
            shared_ptr<Item> shared_item_005( new Item("shared_item_005", 5) );
            shared_ptr<Item> shared_item_006( new Item("shared_item_006", 6) );
            shared_ptr<Item> shared_item_007( new Item("shared_item_007", 7) );
            shared_ptr<Item> shared_item_008( new Item("shared_item_008", 8) );
            shared_ptr<Item> shared_item_009( new Item("shared_item_009", 9) );
            shared_ptr<Item> shared_item_010( new Item("shared_item_010", 10) );
            shared_ptr<Item> shared_item_011( new Item("shared_item_011", 11) );
            shared_ptr<Item> shared_item_012( new Item("shared_item_012", 21) );

            std::set<shared_ptr<Item>> shared_items;
            shared_items.insert( shared_item_001 );
            shared_items.insert( shared_item_002 );
            shared_items.insert( shared_item_003 );
            vector_of_shared_ptr_sets_.push_back( shared_items );

            shared_items.clear();
            shared_items.insert( shared_item_004 );
            shared_items.insert( shared_item_005 );
            shared_items.insert( shared_item_006 );
            vector_of_shared_ptr_sets_.push_back( shared_items );

            shared_items.clear();
            shared_items.insert( shared_item_007 );
            shared_items.insert( shared_item_008 );
            shared_items.insert( shared_item_009 );
            vector_of_shared_ptr_sets_.push_back( shared_items );

            shared_items.clear();
            shared_items.insert( shared_item_010 );
            shared_items.insert( shared_item_011 );
            shared_items.insert( shared_item_012 );
            vector_of_shared_ptr_sets_.push_back( shared_items );

            std::list<weak_ptr<Item>> weak_items;
            weak_items.push_back( shared_item_001 );    
            weak_items.push_back( shared_item_011 );    
            weak_items.push_back( shared_item_011 );
            pre_references_.push_back( weak_items );

            weak_items.clear();
            weak_items.push_back( shared_item_003 );    
            weak_items.push_back( shared_item_004 );    
            weak_items.push_back( shared_item_009 );
            pre_references_.push_back( weak_items );

            weak_items.clear();
            weak_items.push_back( shared_item_012 );    
            weak_items.push_back( shared_item_002 );    
            weak_items.push_back( shared_item_005 );
            pre_references_.push_back( weak_items );

            weak_items.clear();
            weak_items.push_back( shared_item_004 );    
            weak_items.push_back( shared_item_003 );
            weak_items.push_back( shared_item_003 );
            post_references_.push_back( weak_items );

            weak_items.clear();
            weak_items.push_back( shared_item_008 );    
            weak_items.push_back( shared_item_006 );
            weak_items.push_back( shared_item_004 );
            post_references_.push_back( weak_items );
        }

        void clear()
        {
            pre_references_.clear();
            list_of_vectors_.clear();
            vector_of_shared_ptr_sets_.clear();
            post_references_.clear();
        }

        void check( const Model& model ) const
        {           
            CHECK( true );
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.declare <Model> ( "Model", PERSIST_NORMAL );
            archive.declare <Item>  ( "Item",  PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", 1, *this );
            archive.refer( "pre_references",            "pre_reference",   pre_references_ );
            archive.value( "list_of_vectors",           "vector",          list_of_vectors_ );
            archive.value( "vector_of_shared_ptr_sets", "shared_ptr_set",  vector_of_shared_ptr_sets_ );
            archive.refer( "post_references",           "post_reference",  post_references_ );
        }
    };

    TEST( TestContainersInContainersXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestContainersInContainers.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestContainersInContainersJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestContainersInContainers.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestContainersInContainersLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestContainersInContainers.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestContainersInContainersBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestContainersInContainers.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

#endif

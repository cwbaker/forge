//
// TestReordering.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <sweet/persist/set.hpp>
#include <sweet/persist/map.hpp>
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestReordering )
{
    struct Model
    {
        struct OrderedItem
        {
            typedef std::vector<OrderedItem> vector;
            typedef std::vector<std::pair<std::string, OrderedItem> > pair_vector;
            typedef std::set<OrderedItem> set;
            typedef std::map<std::string, OrderedItem> map;

            std::string     name_;
            OrderedItem*    previous_;
            OrderedItem*    next_;

            OrderedItem()
            : name_(),
              previous_(),
              next_()
            {
            }

            OrderedItem( const std::string& name )
            : name_( name ),
              previous_( 0 ),
              next_( 0 )
            {
            }        

            bool operator<( const OrderedItem& item ) const
            {
                return name_ < item.name_;
            }

            template <class Archive> void persist( Archive& archive )
            {
                archive.value( "name",     name_     );
                archive.refer( "previous", previous_ );
                archive.refer( "next",     next_     );
            }
        };

        OrderedItem::vector         items_vector_;
        OrderedItem::pair_vector    items_pair_vector_;
        OrderedItem::set            items_set_;
        OrderedItem::map            items_map_;

        Model()
        : items_vector_(),
          items_set_(),
          items_map_()
        {
            items_vector_.push_back( OrderedItem("4. item-004") );
            items_vector_.push_back( OrderedItem("2. item-002") );            
            items_vector_.push_back( OrderedItem("1. item-001") );
            items_vector_.push_back( OrderedItem("3. item-003") );

            items_vector_[0].previous_ = &items_vector_[3];
            items_vector_[0].next_     = 0;
            items_vector_[1].previous_ = &items_vector_[2];
            items_vector_[1].next_     = &items_vector_[3];
            items_vector_[2].previous_ = 0;
            items_vector_[2].next_     = &items_vector_[1];
            items_vector_[3].previous_ = &items_vector_[1];
            items_vector_[3].next_     = &items_vector_[0];

            items_pair_vector_.push_back( std::make_pair("7. item-007", OrderedItem("7. item-007")) );
            items_pair_vector_.push_back( std::make_pair("8. item-008", OrderedItem("8. item-008")) );
            items_pair_vector_.push_back( std::make_pair("6. item-006", OrderedItem("6. item-006")) );
            items_pair_vector_.push_back( std::make_pair("5. item-005", OrderedItem("5. item-005")) );

            items_pair_vector_[0].second.previous_ = &items_pair_vector_[2].second;
            items_pair_vector_[0].second.next_     = &items_pair_vector_[1].second;
            items_pair_vector_[1].second.previous_ = &items_pair_vector_[0].second;
            items_pair_vector_[1].second.next_     = 0;
            items_pair_vector_[2].second.previous_ = &items_pair_vector_[3].second;
            items_pair_vector_[2].second.next_     = &items_pair_vector_[0].second;
            items_pair_vector_[3].second.previous_ = 0;
            items_pair_vector_[3].second.next_     = &items_pair_vector_[2].second;
        }

        void clear()
        {
            items_vector_.clear();
            items_pair_vector_.clear();
            items_set_.clear();
            items_map_.clear();
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.template declare <OrderedItem> ( "OrderedItem", PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", 1, *this );

            if ( archive.is_writing() )
            {
                archive.value( "set", "item", items_vector_ );
                archive.value( "map", "item", items_pair_vector_ );
            }
            else if ( archive.is_reading() || archive.is_resolving() )
            {
                archive.value( "set", "item", items_set_ );
                archive.value( "map", "item", items_map_ );
            }
        }

        void check( const Model& model )
        {
            std::vector<const OrderedItem*> items;
            for ( OrderedItem::set::const_iterator i = model.items_set_.begin(); i != model.items_set_.end(); ++i )
            {
                items.push_back( &(*i) );
            }

            CHECK( items.size() == 4 );

            if ( items.size() == 4 )
            {
                CHECK( items[0]->name_ == "1. item-001" );
                CHECK( items[0]->previous_ == 0 );
                CHECK( items[0]->next_ == items[1] );
                CHECK( items[1]->name_ == "2. item-002" );
                CHECK( items[1]->previous_ == items[0] );
                CHECK( items[1]->next_ == items[2] );
                CHECK( items[2]->name_ == "3. item-003" );
                CHECK( items[2]->previous_ == items[1] );
                CHECK( items[2]->next_ == items[3] );
                CHECK( items[3]->name_ == "4. item-004" );
                CHECK( items[3]->previous_ == items[2] );
                CHECK( items[3]->next_ == 0 );
            }

            items.clear();
            for ( OrderedItem::map::const_iterator i = model.items_map_.begin(); i != model.items_map_.end(); ++i )
            {
                items.push_back( &i->second );
            }

            CHECK( items.size() == 4 );

            if ( items.size() == 4 )
            {
                CHECK( items[0]->name_ == "5. item-005" );
                CHECK( items[0]->previous_ == 0 );
                CHECK( items[0]->next_ == items[1] );
                CHECK( items[1]->name_ == "6. item-006" );
                CHECK( items[1]->previous_ == items[0] );
                CHECK( items[1]->next_ == items[2] );
                CHECK( items[2]->name_ == "7. item-007" );
                CHECK( items[2]->previous_ == items[1] );
                CHECK( items[2]->next_ == items[3] );
                CHECK( items[3]->name_ == "8. item-008" );
                CHECK( items[3]->previous_ == items[2] );
                CHECK( items[3]->next_ == 0 );
            }        
        }
    };

    TEST( TestReorderingXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestReordering.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestReorderingJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestReordering.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestReorderingLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestReordering.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestReorderingBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestReordering.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

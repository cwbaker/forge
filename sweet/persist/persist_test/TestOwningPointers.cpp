//
// TestOwningPointers.cpp
// Copyright (c) 2008 - 2010 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"

#if defined SWEET_BOOST_ENABLED

#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/auto_ptr.hpp>
#include <sweet/persist/scoped_ptr.hpp>
#include "Item.hpp"
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestOwningPointers )
{
    struct Model
    {
        std::auto_ptr<Item>     auto_ptr_item_;
        boost::scoped_ptr<Item> scoped_ptr_item_;    

    public:
        Model()
        : auto_ptr_item_( new Item("AutoPtrItem", 0) ),
          scoped_ptr_item_( new Item("ScopedPtrItem", 1) )
        {
        }

        void clear()
        {
            auto_ptr_item_.reset();
            scoped_ptr_item_.reset();
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.declare <Item> ( "Item", PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", 1, *this );
            archive.value( "auto_ptr_item",   auto_ptr_item_ );
            archive.value( "scoped_ptr_item", scoped_ptr_item_ );
        }

        void check( const Model& model )
        {
            CHECK( auto_ptr_item_.get() != NULL && model.auto_ptr_item_.get() != NULL && *auto_ptr_item_ == *model.auto_ptr_item_ );
            CHECK( scoped_ptr_item_.get() != NULL && model.scoped_ptr_item_.get() != NULL && *scoped_ptr_item_ == *model.scoped_ptr_item_ );
        }
    };

    TEST( TestOwningPointersXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestOwningPointers.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestOwningPointersJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestOwningPointers.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestOwningPointersLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestOwningPointers.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestOwningPointersBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestOwningPointers.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

#endif

//
// TestLinks.cpp
// Copyright (c) 2008 - 2010 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"

#if defined SWEET_BOOST_ENABLED

#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <sweet/persist/shared_ptr.hpp>
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestLinks )
{
    struct Model;

    struct Link
    {
        typedef boost::shared_ptr<Link> ptr;
        typedef std::vector<Link::ptr>  vector;

        std::string name_;
        Model*      model_;
        
        Link( const std::string& name, Model* model )
        : name_( name ),
          model_( model )
        {
        }

        bool operator==( const Link& resource ) const
        {
            return name_ == resource.name_;
        }

        template <class Archive> void persist( Archive& archive )
        {
        }
    };


    struct Model
    {
        Link::ptr    resource_one_;
        Link::ptr    resource_two_;
        Link::ptr    resource_three_;
        Link::ptr    resource_four_;
        Link::vector resources_;


        Model()
        : resource_one_(),
          resource_two_(),
          resource_three_(),
          resource_four_(),
          resources_()
        {
            resource_one_   = resource( "resource_003" );
            resource_two_   = resource( "resource_002" );
            resource_three_ = resource( "resource_002" );
            resource_four_  = resource( "resource_001" );
        }

        void clear()
        {
            resource_one_.reset();
            resource_two_.reset();
            resource_three_.reset();
            resource_four_.reset();
            resources_.clear();
        }

        Link::ptr resource( const std::string& name )
        {
            Link::ptr resource;

            Link::vector::const_iterator i = resources_.begin();
            while ( i != resources_.end() && (*i)->name_ != name )
            {
                ++i;
            }

            if ( i == resources_.end() )
            {
                resource = Link::ptr( new Link(name, this) );
                resources_.push_back( resource );
            }
            else
            {
                resource = *i;
            }

            return resource;
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.set_context( SWEET_STATIC_TYPEID(Link::ptr), this );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", 1, *this );
            archive.value( "resource_one",   resource_one_ );
            archive.value( "resource_two",   resource_two_ );
            archive.value( "resource_three", resource_three_ );
            archive.value( "resource_four",  resource_four_ );
        }

        void check( const Model& model )
        {
            /*
            CHECK( compare_pointers(resources_, model.resources_) );
            */

            Link::vector::const_iterator i = resources_.begin(); 
            while ( i != resources_.end() && (*i)->model_ == this )
            {
                ++i;
            }

            CHECK( i == resources_.end() );
        }
    };

    template <class Archive> 
    void 
    save( Archive& archive, int mode, const char* name, Link::ptr& resource )
    {
        ObjectGuard<Archive> guard( archive, name, 0, MODE_VALUE, resource != 0 ? 1 : 0 );

        if ( resource.get() != 0 )
        {
            archive.value( "link", resource->name_ );
        }
    }

    template <class Archive> 
    void 
    load( Archive& archive, int mode, const char* name, Link::ptr& resource )
    {
        SWEET_ASSERT( resource.get() == 0 );

        ObjectGuard<Archive> guard( archive, name, 0, mode );
        if ( archive.is_object() )
        {
            std::string link;
            archive.value( "link", link );

            Model* model = reinterpret_cast<Model*>( archive.get_context(SWEET_STATIC_TYPEID(Link::ptr)) );
            SWEET_ASSERT( model != 0 );
            resource = model->resource( link );
        }
    }

    template <class Archive>
    void
    resolve( Archive& archive, int mode, Link::ptr& resource )
    {
        ObjectGuard<Archive> guard( archive, 0, 0, mode );
    }

    TEST( TestLinksXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestLinks.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestLinksJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestLinks.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestLinksLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestLinks.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestLinksBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestLinks.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

#endif

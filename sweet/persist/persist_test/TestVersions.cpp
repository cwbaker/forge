//
// TestVersions.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestVersions )
{
    struct Model
    {
        int         version_;
        std::string version_one_;
        std::string version_two_;
        std::string version_greater_than_one_;
        std::string version_less_than_two_;

        Model()
        : version_( 1 ),
          version_one_( "This string is only persisted if version == 1" ),
          version_two_( "This string is only persisted if version == 2" ),
          version_greater_than_one_( "This string is only persisted if version > 1" ),
          version_less_than_two_( "This string is only persisted if version < 2" )
        {
        }

        Model( int version )
        : version_( version ),
          version_one_( "This string is only persisted if version == 1" ),
          version_two_( "This string is only persisted if version == 2" ),
          version_greater_than_one_( "This string is only persisted if version > 1" ),
          version_less_than_two_( "This string is only persisted if version < 2" )
        {
        }

        void clear()
        {
            version_one_.clear();
            version_two_.clear();
            version_greater_than_one_.clear();
            version_less_than_two_.clear();
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.template declare <Model> ( "Model", PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", version_, *this );

            if ( archive.version() == 1 )
            {
                archive.value( "version_one", version_one_ );
            }
            else
            {
                version_one_.clear();
            }

            if ( archive.version() == 2 )
            {
                archive.value( "version_two", version_two_ );
            }
            else
            {
                version_two_.clear();
            }

            if ( archive.version() > 1 )
            {
                archive.value( "version_greater_than_one", version_greater_than_one_ );
            }
            else
            {
                version_greater_than_one_.clear();
            }

            if ( archive.version() < 2 )
            {
                archive.value( "version_less_than_two", version_less_than_two_ );
            }
            else
            {
                version_less_than_two_.clear();
            }
        }

        void check( const Model& model )
        {
            CHECK( version_ == 1 || version_ == 2 );

            if ( version_ == 1 )
            {
                CHECK( version_one_ == model.version_one_ );
                CHECK( version_two_.empty() );
                CHECK( version_greater_than_one_.empty() );
                CHECK( version_less_than_two_ == model.version_less_than_two_ );
            }
            else if ( version_ == 2 )
            {
                CHECK( version_one_.empty() );
                CHECK( version_two_ == model.version_two_ );
                CHECK( version_greater_than_one_ == model.version_greater_than_one_ );
                CHECK( version_less_than_two_.empty() );
            }
        }
    };

    TEST( TestVersionsXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestVersions.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestVersionsJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestVersions.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestVersionsLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestVersions.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestVersionsBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestVersions.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

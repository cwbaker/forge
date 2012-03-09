//
// TestMissingElements.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestMissingElements )
{
    struct Model
    {
        struct SubElement
        {
            std::string identifier_;

            SubElement()
            : identifier_()
            {
            }

            SubElement( const std::string& identifier )
            : identifier_( identifier )
            {
            }

            bool operator==( const SubElement& element ) const
            {
                return identifier_ == element.identifier_;
            }

            template <class Archive> void persist( Archive& archive )
            {
                archive.value( "identifier", identifier_ );
            }
        };

        struct Element
        {
            std::string identifier_;
            SubElement  sub_element_one_;
            SubElement  sub_element_two_;

            Element()
            : identifier_()
            {
            }

            Element( const std::string& identifier )
            : identifier_( identifier )
            {
            }

            const std::string& get_identifier() const
            {
                return identifier_;
            }

            bool operator==( const Element& item ) const
            {
                return identifier_ == item.identifier_
                    && sub_element_one_ == item.sub_element_one_
                    && sub_element_two_ == item.sub_element_two_
                     ;
            }

            template <class Archive> void persist( Archive& archive )
            {
                archive.value( "identifier",      identifier_      );
                archive.value( "sub_element_one", sub_element_one_ );
                archive.value( "sub_element_two", sub_element_two_ );
            }
        };

        Element not_missing_element_;
        Element missing_element_;
        Element second_not_missing_element_;


        Model()
        : not_missing_element_( "not-missing-element" ),
          missing_element_(),
          second_not_missing_element_( "another-not-missing-element" )
        {
        }

        void clear()
        {
            not_missing_element_        = Element();
            missing_element_            = Element();
            second_not_missing_element_ = Element();
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.template declare <Element> ( "Element", PERSIST_NORMAL );
            archive.template declare <SubElement> ( "SubElement", PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Model", 1, *this );
            archive.value( "not_missing_element",        not_missing_element_ );
            archive.value( "second_not_missing_element", second_not_missing_element_ );

            if ( archive.is_reading() || archive.is_resolving() )
            {
                archive.value( "missing_element", missing_element_ );
            }
        }

        void check( const Model& model )
        {
            CHECK( not_missing_element_ == model.not_missing_element_ );
            CHECK( missing_element_ == model.missing_element_ );
            CHECK( second_not_missing_element_ == model.second_not_missing_element_ );
        }
    };

    TEST( TestMissingElementsXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestMissingElements.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestMissingElementsJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestMissingElements.json" );
        body.written_model_.check( body.read_model_ );
    }

    TEST( TestMissingElementsLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestMissingElements.lua" );
        body.written_model_.check( body.read_model_ );
    }
}

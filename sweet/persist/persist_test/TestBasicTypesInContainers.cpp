//
// TestBasicTypesInContainers.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <sweet/persist/list.hpp>
#include <sweet/persist/set.hpp>
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestBasicTypesInContainers )
{
    struct Model
    {
        std::list<bool> bools_;
        std::vector<char> chars_;
        std::list<signed char> signed_chars_;
        std::set<unsigned char> unsigned_chars_;
        std::vector<wchar_t> wchars_;
        std::list<short> shorts_;
        std::vector<short> unsigned_shorts_;
        std::list<int> ints_;
        std::multiset<unsigned int> unsigned_ints_;
        std::set<long> longs_;
        std::multiset<unsigned long> unsigned_longs_;
        std::vector<float> floats_;
        std::list<double> doubles_;
        std::list<std::string> strings_;
        std::multiset<std::wstring> wstrings_;

        Model()
        {
            bools_.push_back( false );
            bools_.push_back( true );
            bools_.push_back( false );

            chars_.push_back( 'a' );
            chars_.push_back( 'v' );
            chars_.push_back( 'c' );

            signed_chars_.push_back( '$' );
            signed_chars_.push_back( '"' );
            signed_chars_.push_back( '&' );

            unsigned_chars_.insert( 'b' );
            unsigned_chars_.insert( 'l' );
            unsigned_chars_.insert( '<' );
            unsigned_chars_.insert( ';' );
            unsigned_chars_.insert( '>' );

            wchars_.push_back( L'$' );
            wchars_.push_back( L'%' );
            wchars_.push_back( L'&' );
            wchars_.push_back( L'*' );

            ints_.push_back( 2 );
            ints_.push_back( 6 );
            ints_.push_back( 8 );
            ints_.push_back( 3 );
            ints_.push_back( 7 );

            unsigned_ints_.insert( 3 );
            unsigned_ints_.insert( 8 );
            unsigned_ints_.insert( 2 );
            unsigned_ints_.insert( 2 );

            longs_.insert( 32 );
            longs_.insert( 12 );
            longs_.insert( 42 );
            longs_.insert( 62 );

            unsigned_longs_.insert( 123123 );
            unsigned_longs_.insert( 1 );
            unsigned_longs_.insert( 873 );

            floats_.push_back( 5.0f );
            floats_.push_back( 3.34f );
            floats_.push_back( 12.0f );
            floats_.push_back( 321.12f );

            doubles_.push_back( 21.0 );
            doubles_.push_back( 65.2 );
            doubles_.push_back( 12.5 );
            doubles_.push_back( 78.2 );

            strings_.push_back( "this" );
            strings_.push_back( " is " );
            strings_.push_back( "a" );
            strings_.push_back( "test" );

            wstrings_.insert( L"THIS" );
            wstrings_.insert( L"IS" );
            wstrings_.insert( L"THAI" );
            wstrings_.insert( L"ไม่ถือโทษโกรธแช่งซัดฮึดฮัดด่า" );
        }

        void clear()
        {
            bools_.clear();
            chars_.clear();
            signed_chars_.clear();
            unsigned_chars_.clear();
            wchars_.clear();
            shorts_.clear();
            unsigned_shorts_.clear();
            ints_.clear();
            unsigned_ints_.clear();
            longs_.clear();
            unsigned_longs_.clear();
            floats_.clear();
            doubles_.clear();
            strings_.clear();
            wstrings_.clear();
        }

        void check( const Model& model ) const
        {
            CHECK( bools_ == model.bools_ );
            CHECK( chars_ == model.chars_ );
            CHECK( signed_chars_ == model.signed_chars_ );
            CHECK( unsigned_chars_ == model.unsigned_chars_ );
            CHECK( wchars_ == model.wchars_ );
            CHECK( shorts_ == model.shorts_ );
            CHECK( unsigned_shorts_ == model.unsigned_shorts_ );
            CHECK( ints_ == model.ints_ );
            CHECK( unsigned_ints_ == model.unsigned_ints_ );
            CHECK( longs_ == model.longs_ );
            CHECK( unsigned_longs_ == model.unsigned_longs_ );
            CHECK( floats_ == model.floats_ );
            CHECK( doubles_ == model.doubles_ );
            CHECK( strings_ == model.strings_ );
            CHECK( wstrings_ == model.wstrings_ );
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
            archive.enter( "Model", 1, *this );
            archive.value( "bools", "bool", bools_ );
            archive.value( "chars", "char", chars_ );
            archive.value( "signed_chars", "signed_char", signed_chars_ );
            archive.value( "unsigned_chars", "unsigned_char", unsigned_chars_ );
            archive.value( "wchars", "wchar", wchars_ );
            archive.value( "shorts", "short", shorts_ );
            archive.value( "unsigned_shorts", "unsigned_short", unsigned_shorts_ );
            archive.value( "ints", "int", ints_ );
            archive.value( "unsigned_ints", "unsigned_int", unsigned_ints_ );
            archive.value( "longs", "long", longs_ );
            archive.value( "unsigned_longs", "unsigned_long", unsigned_longs_ );
            archive.value( "floats", "float", floats_ );
            archive.value( "doubles", "double", doubles_ );
            archive.value( "strings", "string", strings_ );
            archive.value( "wstrings", "wstring", wstrings_ );
        }
    };

    TEST( TestBasicTypesInContainersXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestBasicTypesInContainers.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestBasicTypesInContainersJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestBasicTypesInContainers.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestBasicTypesInContainersLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestBasicTypesInContainers.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestBasicTypesInContainersBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestBasicTypesInContainers.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

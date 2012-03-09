//
// TestBasicTypes.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include "TestBody.hpp"

using namespace sweet::persist;

SUITE( TestBasicTypes )
{
    struct Model
    {
        char            char_;
        signed char     signed_char_;
        unsigned char   unsigned_char_;
        wchar_t         wchar_;
        short           short_;
        unsigned short  unsigned_short_;
        int             int_;
        unsigned int    unsigned_int_;
        long            long_;
        unsigned long   unsigned_long_;
        float           float_;
        double          double_;
        std::wstring    string_;
        std::string     narrow_string_;
        wchar_t         short_fixed_length_string_ [16];
        char            short_fixed_length_narrow_string_ [16];
        wchar_t         fixed_length_string_ [32];
        char            fixed_length_narrow_string_ [32];

        Model()
        : char_( 'C' ),
          signed_char_( 'c' ),
          unsigned_char_( 'z' ),
          wchar_( L'S' ),
          short_( -23 ),
          unsigned_short_( 32 ),
          int_( -3221 ),
          unsigned_int_( 2133 ),
          long_( 21 ),
          unsigned_long_( 32 ),
          float_( 1.432f ),
          double_( 0.12321 ),
          string_( L"wide character string" ),
          narrow_string_( "narrow character string" )
        {
            wcsncpy( short_fixed_length_string_, L"short fixed length string", sizeof(short_fixed_length_string_) / sizeof(wchar_t) );
            strncpy( short_fixed_length_narrow_string_, "short fixed length narrow string", sizeof(short_fixed_length_narrow_string_) );
            wcsncpy( fixed_length_string_, L"fixed length string", sizeof(fixed_length_string_) / sizeof(wchar_t) );
            strncpy( fixed_length_narrow_string_, "fixed length narrow string", sizeof(fixed_length_narrow_string_) );
        }

        void clear()
        {
            char_              = 0;
            signed_char_       = 0;
            unsigned_char_     = 0;
            wchar_             = 0;
            short_             = 0;
            unsigned_short_    = 0;
            int_               = 0;
            unsigned_int_      = 0;
            long_              = 0;
            unsigned_long_     = 0;
            float_             = 0;
            double_            = 0;
            string_.clear();
            narrow_string_.clear();
            memset( &short_fixed_length_string_, 0, sizeof(short_fixed_length_string_) );
            memset( &short_fixed_length_narrow_string_, 0, sizeof(short_fixed_length_narrow_string_) );
            memset( &fixed_length_string_, 0, sizeof(fixed_length_string_) );
            memset( &fixed_length_narrow_string_, 0, sizeof(fixed_length_narrow_string_) );
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

            if ( archive.is_writing() )
            {
                archive.value( "short_fixed_length_string",         short_fixed_length_string_,        sizeof(short_fixed_length_string_) / sizeof(wchar_t) );
                archive.value( "short_fixed_length_narrow_string",  short_fixed_length_narrow_string_, sizeof(short_fixed_length_narrow_string_) );
            }
            else if ( archive.is_reading() )
            {
                archive.value( "short_fixed_length_string",         short_fixed_length_string_,        (size_t) 4 );
                archive.value( "short_fixed_length_narrow_string",  short_fixed_length_narrow_string_, (size_t) 4 );
            }

            archive.value( "char",                       char_ );
            archive.value( "signed_char",                signed_char_ );
            archive.value( "unsigned_char",              unsigned_char_ );
            archive.value( "wchar",                      wchar_ );
            archive.value( "short",                      short_ );
            archive.value( "unsigned_short",             unsigned_short_ );
            archive.value( "int",                        int_ );
            archive.value( "unsigned_int",               unsigned_int_ );
            archive.value( "long",                       long_ );
            archive.value( "unsigned_long",              unsigned_long_ );
            archive.value( "float",                      float_ );
            archive.value( "double",                     double_ );
            archive.value( "string",                     string_ );
            archive.value( "narrow_string",              narrow_string_ );
            archive.value( "fixed_length_string",        fixed_length_string_, sizeof(fixed_length_string_) / sizeof(wchar_t) );
            archive.value( "fixed_length_narrow_string", fixed_length_narrow_string_, sizeof(fixed_length_narrow_string_) );
        }

        void check( const Model& model )
        {
            CHECK( char_ == model.char_ );
            CHECK( signed_char_ == model.signed_char_ );
            CHECK( unsigned_char_ == model.unsigned_char_ );
            CHECK( wchar_ == model.wchar_ );
            CHECK( short_ == model.short_ );
            CHECK( unsigned_short_ == model.unsigned_short_ );
            CHECK( int_ == model.int_ );
            CHECK( unsigned_int_ == model.unsigned_int_ );
            CHECK( long_ == model.long_ );
            CHECK( unsigned_long_ == model.unsigned_long_ );
            CHECK_CLOSE( float_, model.float_, FLT_EPSILON );
            CHECK_CLOSE( double_, model.double_, DBL_EPSILON );
            CHECK( string_ == model.string_ );
            CHECK( narrow_string_ == model.narrow_string_ );
        }
    };

    TEST( TestBasicTypesXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestBasicTypes.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestBasicTypesJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestBasicTypes.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestBasicTypesLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestBasicTypes.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestBasicTypesBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestBasicTypes.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

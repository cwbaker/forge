//
// TestPathFilter.cpp
// Copyright (c) 2008 - 2010 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"

#if defined SWEET_BOOST_ENABLED

#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <sweet/persist/list.hpp>
#include <sweet/persist/shared_ptr.hpp>
#include "TestBody.hpp"

using namespace sweet;
using namespace sweet::persist;

SUITE( TestPathFilter )
{
    struct Model
    {
        struct LinkedItem
        {
            typedef boost::shared_ptr<LinkedItem> ptr;
            typedef std::list<ptr>                list;

            std::string identifier_;

            LinkedItem( const std::string& identifier )
            : identifier_( identifier )
            {
            }

            const std::string& get_identifier() const
            {
                return identifier_;
            }

            bool operator==( const LinkedItem& item ) const
            {
                return identifier_ == item.identifier_;
            }
        };


        template <class Char>
        struct PathItem
        {
            typedef std::vector<PathItem> vector;
            typedef std::list<PathItem*>  list;

            std::basic_string<Char> identifier_;

            PathItem()
            : identifier_()
            {
            }

            PathItem( const std::basic_string<Char>& identifier )
            : identifier_( identifier )
            {
            }

            bool operator==( const PathItem& item ) const
            {
                return identifier_ == item.identifier_;
            }

            template <class Archive> void persist( Archive& archive )
            {
                archive.value( "identifier", identifier_, path_filter(archive.get_path()) );
            }
        };

        std::string                  path_;
        std::wstring                 wide_path_;
        std::wstring                 path_on_different_drive_;
        char                         fixed_length_path_ [64];
        wchar_t                      fixed_length_wide_path_ [64];
        std::wstring                 empty_path_item_;
        PathItem<char>::vector       path_items_;  
        PathItem<wchar_t>::vector    wide_path_items_;
        PathItem<char>::list         pre_references_;
        PathItem<char>::list         post_references_;
        PathItem<wchar_t>::list      wide_pre_references_;
        PathItem<wchar_t>::list      wide_post_references_;
        LinkedItem::list             linked_path_items_;

        Model()
        : path_(),
          wide_path_(),
          empty_path_item_(),
          path_items_(),
          wide_path_items_(),
          pre_references_(),
          post_references_(),
          wide_pre_references_(),
          wide_post_references_(),
          linked_path_items_()
        {
            path::WidePath cwd = path::current_working_directory();
            path_      = narrow(cwd.string()) + "/bollix.txt";
            wide_path_ = cwd.string() + L"/bar.html";
            path_on_different_drive_ = L"Z:/another/drive.txt";

            strncpy( fixed_length_path_,      path_.c_str(),      sizeof(fixed_length_path_) );
            fixed_length_path_[sizeof(fixed_length_path_) - 1] = 0;
            wcsncpy( fixed_length_wide_path_, wide_path_.c_str(), sizeof(fixed_length_wide_path_) / sizeof(wchar_t) );
            fixed_length_wide_path_[sizeof(fixed_length_wide_path_) / sizeof(wchar_t) - 1] = 0;

            path_items_.push_back( PathItem<char>(narrow(cwd.string()) + "/hello") );
            path_items_.push_back( PathItem<char>(narrow(cwd.string()) + "/there") );
            path_items_.push_back( PathItem<char>(narrow(cwd.string()) + "/cruel") );
            path_items_.push_back( PathItem<char>(narrow(cwd.string()) + "/world") );

            wide_path_items_.push_back( PathItem<wchar_t>(cwd.string() + L"/HELLO") );
            wide_path_items_.push_back( PathItem<wchar_t>(cwd.string() + L"/THERE") );
            wide_path_items_.push_back( PathItem<wchar_t>(cwd.string() + L"/CRUEL") );
            wide_path_items_.push_back( PathItem<wchar_t>(cwd.string() + L"/WORLD") );

            pre_references_.push_back( &path_items_[3] );
            pre_references_.push_back( &path_items_[0] );
            pre_references_.push_back( &path_items_[1] );
            pre_references_.push_back( &path_items_[3] );

            post_references_.push_back( &path_items_[1] );
            post_references_.push_back( &path_items_[1] );
            post_references_.push_back( &path_items_[2] );
            post_references_.push_back( &path_items_[3] );

            wide_pre_references_.push_back( &wide_path_items_[2] );
            wide_pre_references_.push_back( &wide_path_items_[3] );
            wide_pre_references_.push_back( &wide_path_items_[0] );
            wide_pre_references_.push_back( &wide_path_items_[1] );

            wide_post_references_.push_back( &wide_path_items_[2] );
            wide_post_references_.push_back( &wide_path_items_[3] );
            wide_post_references_.push_back( &wide_path_items_[3] );
            wide_post_references_.push_back( &wide_path_items_[1] );

            linked_path_items_.push_back( link(narrow(cwd.string() + L"/this/is/a/path.txt")) );
            linked_path_items_.push_back( link("Z:/on/another/drive/somewhere.txt") );
        }

        void clear()
        {
            path_.clear();
            wide_path_.clear();
            path_on_different_drive_.clear();
            memset( fixed_length_path_, 0, sizeof(fixed_length_path_) );
            memset( fixed_length_wide_path_, 0, sizeof(fixed_length_wide_path_) );
            path_items_.clear();
            wide_path_items_.clear();
            pre_references_.clear();
            post_references_.clear();
            wide_pre_references_.clear();
            wide_post_references_.clear();
            linked_path_items_.clear();
        }

        LinkedItem::ptr link( const std::string& identifier )
        {
            return LinkedItem::ptr( new LinkedItem(identifier) );
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.declare <PathItem<char>>    ( "PathItem",     PERSIST_NORMAL );
            archive.declare <PathItem<wchar_t>> ( "WidePathItem", PERSIST_NORMAL );
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
                archive.enter( "Model", 1, *this );

                archive.refer( "pre_references",      "pre_reference",      pre_references_ );
                archive.refer( "wide_pre_references", "wide_pre_reference", wide_pre_references_ );

                archive.value( "path",                    path_,      path_filter(archive.get_path()) );
                archive.value( "wide_path",               wide_path_, path_filter(archive.get_path()) );
                archive.value( "path_on_different_drive", path_on_different_drive_, path_filter(archive.get_path()) );
                archive.value( "fixed_length_path",       fixed_length_path_,      sizeof(fixed_length_path_),                        path_filter(archive.get_path()) );
                archive.value( "fixed_length_wide_path",  fixed_length_wide_path_, sizeof(fixed_length_wide_path_) / sizeof(wchar_t), path_filter(archive.get_path()) );
                archive.value( "empty_path_item", empty_path_item_, path_filter(archive.get_path()) );
                archive.value( "path_items",      "path_item",      path_items_ );
                archive.value( "wide_path_items", "wide_path_item", wide_path_items_ );

                archive.refer( "post_references",      "post_reference",      post_references_ );
                archive.refer( "wide_post_references", "wide_post_reference", wide_post_references_ );
        }

        void check( const Model& model )
        {
            CHECK( path_ == model.path_ );
            CHECK( wide_path_ == model.wide_path_ );
            CHECK( path_on_different_drive_ == model.path_on_different_drive_ );
            CHECK( strcmp(fixed_length_path_, model.fixed_length_path_) == 0 );
            CHECK( wcscmp(fixed_length_wide_path_, model.fixed_length_wide_path_) == 0 );
            CHECK( empty_path_item_ == model.empty_path_item_ );
            /*
            CHECK( compare(path_items_ ) );
            CHECK( compare(wide_path_items_ ) );
            CHECK( compare_pointers(pre_references_ ) );
            CHECK( compare_pointers(wide_pre_references_ ) );
            CHECK( compare_pointers(post_references_ ) );
            CHECK( compare_pointers(wide_post_references_ ) );
            */
        }
    };

    TEST( TestPathFilterXml )
    {
        TestBody<XmlWriter, XmlReader, Model> body( "TestPathFilter.xml" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestPathFilterJson )
    {
        TestBody<JsonWriter, JsonReader, Model> body( "TestPathFilter.json" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestPathFilterLua )
    {
        TestBody<LuaWriter, LuaReader, Model> body( "TestPathFilter.lua" );
        body.written_model_.check( body.read_model_ );
    }
    
    TEST( TestPathFilterBinary )
    {
        TestBody<BinaryWriter, BinaryReader, Model> body( "TestPathFilter.dat" );
        body.written_model_.check( body.read_model_ );
    }
}

#endif

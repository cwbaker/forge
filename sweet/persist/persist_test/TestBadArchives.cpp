//
// TestBadArchives.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include "Item.hpp"

using namespace sweet;
using namespace sweet::persist;

SUITE( TestBadArchives )
{
    struct Model
    {
        std::vector<Item> items_;

        Model()
        {
            items_.insert( items_.end(), 4, Item() );
            items_[0].m_string = "item_000";
            items_[1].m_string = "item_001";
            items_[2].m_string = "item_002";
            items_[3].m_string = "item_003";
        }

        void clear()
        {
            items_.clear();
        }

        template <class Archive> void enter( Archive& archive )
        {
        }

        template <class Archive> void exit( Archive& archive )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.value( "items", "item", items_ );
        }
    };

    TEST( TestXmlBadArchive1 )
    {
        Model read_model;
        read_model.clear();
        XmlReader xml_reader;
        CHECK_THROW( xml_reader.read("TestXmlBadArchive1.xml", "model", read_model), persist::Error );
    }

    TEST( TestXmlBadArchive2 )
    {
        Model read_model;
        read_model.clear();
        XmlReader xml_reader;
        CHECK_THROW( xml_reader.read("TestXmlBadArchive2.xml", "model", read_model), persist::Error );
    }

    TEST( TestXmlBadArchive3 )
    {
        Model read_model;
        read_model.clear();
        XmlReader xml_reader;
        CHECK_THROW( xml_reader.read("TestXmlBadArchive3.xml", "model", read_model), persist::Error );
    }

    TEST( TestXmlBadArchive4 )
    {
        Model read_model;
        read_model.clear();
        XmlReader xml_reader;
        CHECK_THROW( xml_reader.read("TestXmlBadArchive4.xml", "model", read_model), persist::Error );
    }
}

//
// Item.hpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef ITEM_HPP_INCLUDED
#define ITEM_HPP_INCLUDED

namespace sweet
{

namespace persist
{

enum ItemType
{
    ITEM_TYPE_NORMAL,
    ITEM_TYPE_SMALL,
    ITEM_TYPE_LARGE
};

extern const EnumFilter::Conversion ITEM_TYPES[];

enum ItemFlags
{
    ITEM_FLAG_NONE = 0x00,
    ITEM_FLAG_SELECTED = 0x01,
    ITEM_FLAG_EXPANDED = 0x02,
    ITEM_FLAG_DIRTY = 0x04
};

extern const MaskFilter::Conversion ITEM_FLAGS[];

struct Item
{
    std::string   m_string;
    std::wstring  m_wstring;
    bool m_boolean;
    int m_integer;
    float m_real;
    ItemType m_type;
    int m_flags;
    int m_references;

    public:
        Item();
        Item( const std::string& string, int integer );
        Item& operator=( const Item& item );

        void clear();
        bool operator==( const Item& item ) const;
        bool operator!=( const Item& item ) const;
        bool operator<( const Item& item ) const;
        operator size_t() const;

        template <class Archive> void enter( Archive& archive );
        template <class Archive> void exit( Archive& archive );
        template <class Archive> void persist( Archive& archive );
};

void intrusive_ptr_add_ref( Item* item );
void intrusive_ptr_release( Item* item );

}

}

template <class Archive>
void sweet::persist::Item::enter( Archive& archive )
{
    archive.template declare( "Item", PERSIST_NORMAL );
}

template <class Archive>
void sweet::persist::Item::exit( Archive& archive )
{
}

template <class Archive> 
void sweet::persist::Item::persist( Archive& archive )
{
    archive.value( "string",  m_string  );
    archive.value( "wstring", m_wstring );
    archive.value( "boolean", m_boolean );
    archive.value( "integer", m_integer );
    archive.value( "real", m_real );
    archive.value( "type", m_type, enum_filter(ITEM_TYPES) );
    archive.value( "flags", m_flags, mask_filter(ITEM_FLAGS) );
}

#endif

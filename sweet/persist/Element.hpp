//
// Element.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_ELEMENT_HPP_INCLUDED
#define SWEET_PERSIST_ELEMENT_HPP_INCLUDED

#include "declspec.hpp"
#include "Attribute.hpp"
#include <string>
#include <list>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// An element that stores attributes and other elements.
*/
class SWEET_PERSIST_DECLSPEC Element
{
    std::string m_name; ///< The name of the Element.
    const void* m_address; ///< The address of the object of this Element.
    int m_flags; ///< The flags for this Element.
    std::list<Attribute> m_attributes; ///< The Attributes.
    std::list<Element> m_elements; ///< The child Elements.

    public:
        Element();
        Element( const std::string& name, const void* address );

        const std::string& name() const;
        const void* address() const;
        bool is_leaf() const;

        void set_flag( int flag );
        bool is_flag( int flag ) const;

        std::list<Attribute>& attributes();
        const std::list<Attribute>& attributes() const;

        std::list<Element>& elements();
        const std::list<Element>& elements() const;

        void clear();

        void set_type( const std::string& type );
        Element* add_element( const Element& element );
        void add_attribute( const Attribute& attribute );

        Attribute* find_attribute( const std::string& name );
        Element* find_element( const std::string& name );
};

}

}

#endif

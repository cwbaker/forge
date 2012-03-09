//
// Address.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_ADDRESS_HPP_INCLUDED
#define SWEET_PERSIST_ADDRESS_HPP_INCLUDED

#include "declspec.hpp"

namespace sweet
{

namespace persist
{

class SWEET_PERSIST_DECLSPEC Address
{
    const void* m_address; ///< The identifier.
    void* m_raw_ptr; ///< The address of the identified object.
    void* m_smart_ptr; ///< The address of the smart pointer that manages the object (if any).

    public:
        Address( const void* address, void* raw_ptr, void* smart_ptr );
        const void* address() const;
        void* raw_ptr() const;
        void* smart_ptr() const;
        bool operator<( const Address& address ) const;
};

}

}

#endif

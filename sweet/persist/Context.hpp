//
// Context.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_CONTEXT_HPP_INCLUDED
#define SWEET_PERSIST_CONTEXT_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/rtti/Type.hpp>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// Associates a type (by rtti::Type) with client supplied context 
// information.
*/
class SWEET_PERSIST_DECLSPEC Context
{
    rtti::Type m_type;
    void* m_context;

    public:
        Context( const rtti::Type& type, void* context );
        void* get_context() const;
        bool operator<( const Context& context ) const;
};

}

}

#endif

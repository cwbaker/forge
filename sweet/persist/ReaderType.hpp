//
// ReaderType.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_READERTYPE_HPP_INCLUDED
#define SWEET_PERSIST_READERTYPE_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/rtti/Type.hpp>
#include <string>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// A type that can be created and persisted when reading an archive.
*/
template <class Archive>
class ReaderType
{
    public:
        struct less_by_name
        {
            bool operator()( const ReaderType* lhs, const ReaderType* rhs ) const
            {
                SWEET_ASSERT( lhs && rhs );
                return lhs->compare_by_name( *rhs );
            }
        };

        typedef std::set<ReaderType> set;
        typedef std::set<const ReaderType*, less_by_name> set_by_name;

    private:
        rtti::Type m_type; ///< The rtti::Type for the type returned by SWEET_TYPEID().
        std::string m_name; ///< The application supplied name for the type.
        int m_flags; ///< Whether or not the type is polymorphic.
        void* (*m_create)(); ///< The function to create an object of the type on the heap.
        void (*m_persist)( Archive&, void* ); ///< The function to persist an object of the type.

    public:
        ReaderType( const rtti::Type& type, const std::string& name, int flags, void* (*create)(), void (*persist)(Archive&, void*) );
        bool is_polymorphic() const;
        bool is_anonymous() const;
        void* create() const;
        void persist( Archive& archive, void* object ) const;
        bool operator<( const ReaderType& type ) const;
        bool compare_by_name( const ReaderType& type ) const;
};

}

}

#endif


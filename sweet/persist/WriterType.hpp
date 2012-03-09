//
// WriterType.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_WRITERTYPE_HPP_INCLUDED
#define SWEET_PERSIST_WRITERTYPE_HPP_INCLUDED

#include <sweet/rtti/Type.hpp>
#include <string>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// A named type that can be persisted when writing an archive.
*/
template <class Archive>
class WriterType
{
    rtti::Type m_type; ///< The rtti::Type for the type returned by SWEET_TYPEID().
    std::string m_name; ///< The application supplied name for the type.
    int m_flags; ///< Whether or not the type is polymorphic.
    void (*m_persist)( Archive&, void* ); ///< The function to persist an object of the type.

    public:
        WriterType( const rtti::Type& type, const std::string& name, int flags, void (*persist)(Archive&, void*) );
        const std::string& get_name() const;
        bool is_polymorphic() const;
        bool is_anonymous() const;
        void persist( Archive& archive, void* object ) const;
        bool operator<( const WriterType& type ) const;
};

}

}

#endif

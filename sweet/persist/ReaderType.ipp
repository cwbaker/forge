//
// ReaderType.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_READERTYPE_IPP_INCLUDED
#define SWEET_PERSIST_READERTYPE_IPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Archive>
ReaderType<Archive>::ReaderType( const rtti::Type& type, const std::string& name, int flags, void* (*create)(), void (*persist)(Archive&, void*) )
: m_type( type ),
  m_name( name ),
  m_flags( flags ),
  m_create( create ),
  m_persist( persist )
{
}

template <class Archive>
bool ReaderType<Archive>::is_polymorphic() const
{
    return (m_flags & PERSIST_POLYMORPHIC) != 0;
}

template <class Archive>
void* ReaderType<Archive>::create() const
{
    SWEET_ASSERT( m_create );
    return (*m_create)();
}

template <class Archive>
void ReaderType<Archive>::persist( Archive& archive, void* object ) const
{
    SWEET_ASSERT( m_persist );
    (*m_persist)( archive, object );
}

template <class Archive>
bool ReaderType<Archive>::operator<( const ReaderType& type ) const
{
    return m_type < type.m_type;
}

template <class Archive>
bool ReaderType<Archive>::compare_by_name( const ReaderType& type ) const
{
    return m_name < type.m_name;
}

}

}

#endif

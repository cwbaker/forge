//
// WriterType.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_WRITERTYPE_IPP_INCLUDED
#define SWEET_PERSIST_WRITERTYPE_IPP_INCLUDED

namespace sweet
{

namespace persist
{

template <class Archive>
WriterType<Archive>::WriterType( const rtti::Type& type, const std::string& name, int flags, void (*persist)(Archive&, void*) )
: m_type( type ),
  m_name( name ),
  m_flags( flags ),
  m_persist( persist )
{
}

template <class Archive>
const std::string& WriterType<Archive>::get_name() const
{
    return m_name;
}

template <class Archive>
bool WriterType<Archive>::is_polymorphic() const
{
    return (m_flags & PERSIST_POLYMORPHIC) != 0;
}

template <class Archive>
void WriterType<Archive>::persist( Archive& archive, void* object ) const
{
    SWEET_ASSERT( m_persist );
    (*m_persist)( archive, object );
}

template <class Archive>
bool WriterType<Archive>::operator<( const WriterType& type ) const
{
    return m_type < type.m_type;
}

}

}

#endif

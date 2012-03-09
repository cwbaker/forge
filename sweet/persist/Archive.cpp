//
// Archive.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Archive.hpp"

using std::set;
using namespace sweet;
using namespace sweet::persist;

Archive::Archive( ArchiveType type )
: m_type( type ),
  m_filename(),
  m_path(),
  m_entered( false ),
  m_version( 0 ),
  m_format_keyword( "format" ),
  m_version_keyword( "version" ),
  m_class_keyword( "class" ),
  m_address_keyword( "address" ),
  m_contexts()
{
}

Archive::Archive( const Archive& archive )
: m_type( archive.m_type ),
  m_filename(),
  m_path(),
  m_entered( false ),
  m_version( 0 ),
  m_format_keyword( archive.m_format_keyword ),
  m_version_keyword( archive.m_version_keyword ),
  m_class_keyword( archive.m_class_keyword ),
  m_address_keyword( archive.m_address_keyword ),
  m_contexts( archive.m_contexts )
{
}

bool Archive::is_writing() const
{
    return m_type == ARCHIVE_WRITER;
}

bool Archive::is_reading() const
{
    return m_type == ARCHIVE_READER;
}

bool Archive::is_resolving() const
{
    return m_type == ARCHIVE_RESOLVER;
}

void Archive::set_filename( const std::wstring& filename )
{
    m_filename = filename;

    path::WidePath path( filename );
    if ( path.is_relative() )
    {
        path = path::current_working_directory() / path;
    }
    m_path = path.branch();
    m_path.normalize();
}

const std::wstring& Archive::get_filename() const
{
    return m_filename;
}

const sweet::path::WidePath& Archive::get_path() const
{
    return m_path;
}

void Archive::set_entered( bool entered )
{
    m_entered = entered;
}

bool Archive::is_entered() const
{
    return m_entered;
}

void Archive::set_version( int version )
{
    m_version = version;
}

int Archive::version() const
{
    return m_version;
}

void Archive::set_format_keyword( const char* format_keyword )
{
    m_format_keyword = format_keyword;
}

const std::string& Archive::get_format_keyword() const
{
    return m_format_keyword;
}

void Archive::set_version_keyword( const char* version_keyword )
{
    m_version_keyword = version_keyword;
}

const std::string& Archive::get_version_keyword() const
{
    return m_version_keyword;
}

void Archive::set_class_keyword( const char* class_keyword )
{
    SWEET_ASSERT( class_keyword );   
    m_class_keyword = class_keyword;
    SWEET_ASSERT( !m_class_keyword.empty() );
}

const std::string& Archive::get_class_keyword() const
{
    return m_class_keyword;
}

void Archive::set_address_keyword( const char* address_keyword )
{
    SWEET_ASSERT( address_keyword );
    m_address_keyword = address_keyword;
    SWEET_ASSERT( !m_address_keyword.empty() );
}

const std::string& Archive::get_address_keyword() const
{
    return m_address_keyword;
}

void Archive::set_context( const rtti::Type& type, void* context )
{
    m_contexts.insert( Context(type, context) );
}

void* Archive::get_context( const rtti::Type& type ) const
{
    set<Context>::const_iterator i = m_contexts.find( Context(type, 0) );
    return i != m_contexts.end() ? i->get_context() : NULL;
}

void Archive::set_contexts( const std::set<Context>& contexts )
{
    m_contexts = contexts;
}

const std::set<Context>& Archive::get_contexts() const
{
    return m_contexts;
}

//
// PathFilter.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "PathFilter.hpp"
#include "functions.hpp"

using namespace sweet::persist;

PathFilter::PathFilter( const sweet::path::WidePath& path )
: m_path( path )
{
}

std::string PathFilter::to_memory( const std::string& value ) const
{
    path::WidePath path( widen(value) );
    if ( !path.empty() && !path.is_absolute() )
    {
        path = m_path / path;
        path.normalize();
    }
    return narrow( path.string() );
}

std::wstring PathFilter::to_memory( const std::wstring& value ) const
{
    path::WidePath path( value );
    if ( !path.empty() && !path.is_absolute() )
    {
        path = m_path / path;
        path.normalize();
    }
    return path.string();
}

std::string PathFilter::to_archive( const std::string& value ) const
{
    SWEET_ASSERT( value.empty() || path::WidePath(widen(value)).is_absolute() );
    
    path::WidePath relative_path;
    if ( !value.empty() )
    {
        relative_path = m_path.relative( path::WidePath(widen(value)) );
    }
    return narrow( relative_path.string() );
}

std::wstring PathFilter::to_archive( const std::wstring& value ) const
{
    SWEET_ASSERT( value.empty() || path::WidePath(value).is_absolute() );
 
    path::WidePath relative_path;
    if ( !value.empty() )
    {
        relative_path = m_path.relative( path::WidePath(value) );
    }
    return relative_path.string();
}

PathFilter sweet::persist::path_filter( const sweet::path::WidePath& path )
{
    return PathFilter( path );
}

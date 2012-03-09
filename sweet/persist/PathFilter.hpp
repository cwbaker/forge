//
// PathFilter.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_PATHFILTER_HPP_INCLUDED
#define SWEET_PERSIST_PATHFILTER_HPP_INCLUDED

#include "declspec.hpp"
#include <sweet/path/Path.hpp>

namespace sweet
{

namespace persist
{

/**
// @internal
//
// A simple filter that converts from absolute paths in memory to paths 
// relative to the archive path in an archive.
*/
class SWEET_PERSIST_DECLSPEC PathFilter
{
    const path::WidePath& m_path; ///< The path to make paths relative to.

    public:
        PathFilter( const path::WidePath& path );
        std::string to_memory( const std::string& value ) const;
        std::wstring to_memory( const std::wstring& value ) const;
        std::string to_archive( const std::string& value ) const;
        std::wstring to_archive( const std::wstring& value ) const;
};

SWEET_PERSIST_DECLSPEC PathFilter path_filter( const path::WidePath& path );

}

}

#endif


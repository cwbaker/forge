//
// Archive.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_ARCHIVE_HPP_INCLUDED
#define SWEET_PERSIST_ARCHIVE_HPP_INCLUDED

#include "declspec.hpp"
#include "Context.hpp"
#include <sweet/path/Path.hpp>
#include <string>
#include <set>

namespace sweet
{

namespace persist
{

/**
// The type of an Archive.
*/
enum ArchiveType
{
    ARCHIVE_WRITER,     ///< The Archive is a Writer.
    ARCHIVE_READER,     ///< The Archive is a Reader.
    ARCHIVE_RESOLVER    ///< The Archive is a Resolver.
};

struct ArchiveTypeWriter {};
struct ArchiveTypeReader {};
struct ArchiveTypeResolver {};

/**
// Base class for Archives.
//
// Provides functionality common to all archives - the keywords for various
// attributes, the filename, path, format, version, and context information
// that has been stored against the Archive.
*/
class SWEET_PERSIST_DECLSPEC Archive
{
    ArchiveType m_type; ///< The type of this Archive.
    std::wstring m_filename; ///< The filename of this Archive.
    path::WidePath m_path; ///< The path to this Archive.
    bool m_entered; ///< Whether or not this Archive has been entered.
    int m_version; ///< The version of the Archive.
    std::string m_format_keyword; ///< The keyword used to name the attribute that stores the format of an Archive.
    std::string m_version_keyword; ///< The keyword used to name the attribute that stores the version of an Archive.
    std::string m_class_keyword; ///< The keyword used to name the attribute that stores the class of an object.
    std::string m_address_keyword; ///< The keyword used to name the attribute that stores the address of an object.
    std::set<Context> m_contexts; ///< The associations between types and client provided context information.

    public:
        Archive( ArchiveType type );
        Archive( const Archive& archive );

        bool is_writing() const;
        bool is_reading() const;
        bool is_resolving() const;

        void set_filename( const std::wstring& filename );
        const std::wstring& get_filename() const;
        const path::WidePath& get_path() const;

        void set_entered( bool entered );
        bool is_entered() const;

        void set_version( int version );
        int version() const;

        void set_format_keyword( const char* format_keyword );
        const std::string& get_format_keyword() const;

        void set_version_keyword( const char* version_keyword );
        const std::string& get_version_keyword() const;

        void set_class_keyword( const char* class_keyword );
        const std::string& get_class_keyword() const;

        void set_address_keyword( const char* address_keyword );
        const std::string& get_address_keyword() const;

        void set_context( const rtti::Type& type, void* context );
        void* get_context( const rtti::Type& type ) const;

        void set_contexts( const std::set<Context>& contexts );
        const std::set<Context>& get_contexts() const;
};

}

}

#endif

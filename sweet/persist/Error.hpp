//
// Error.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_ERROR_HPP_INCLUDED
#define SWEET_PERSIST_ERROR_HPP_INCLUDED

#include "declspec.hpp"
#include "Reference.hpp"
#include <sweet/error/Error.hpp>
#include <sweet/error/ErrorTemplate.hpp>
#include <set>

namespace sweet
{

namespace persist
{

/**
// Error codes for exceptions thrown from the %persist library.
*/
enum ErrorCodes
{
    PERSIST_ERROR_NONE, ///< No error has occured.
    PERSIST_ERROR_OPENING_FILE_FAILED, ///< Opening a file failed.
    PERSIST_ERROR_READING_FILE_FAILED, ///< Reading a file failed.
    PERSIST_ERROR_PARSING_FILE_FAILED, ///< Parsing a file failed.
    PERSIST_ERROR_WRITING_FILE_FAILED, ///< Writing a file failed.
    PERSIST_ERROR_INVALID_TYPE, ///< A type that has not been declared has been encountered.
    PERSIST_ERROR_INVALID_VERSION, ///< The version of an archive didn't match that expected by the application.
    PERSIST_ERROR_INVALID_FORMAT, ///< The format of an archive didn't match that expected by the application.
    PERSIST_ERROR_INVALID_IDENTIFIER, ///< An identifier for an enumerated or mask value wasn't able to be converted to an integral value.
    PERSIST_ERROR_UNRESOLVED_REFERENCES  ///< Not all references in an archive were resolved.
};

/**
// Errors thrown from the %persist library.
*/
class SWEET_PERSIST_DECLSPEC Error : public error::Error 
{
    public:
        Error( int error );
};

/**
// Opening a file failed.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_OPENING_FILE_FAILED, Error> OpeningFileFailedError;

/**
// Reading a file failed.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_READING_FILE_FAILED, Error> ReadingFileFailedError;

/**
// Parsing a file failed.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_PARSING_FILE_FAILED, Error> ParsingFileFailedError;

/**
// Writing a file failed.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_WRITING_FILE_FAILED, Error> WritingFileFailedError;

/**
// A type that has not been declared has been encountered.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_INVALID_TYPE, Error> InvalidTypeError;

/**
// The version of an Archive doesn't match that expected by the application.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_INVALID_VERSION, Error> InvalidVersionError;

/**
// The format of an Archive doesn't match that expected by the application.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_INVALID_FORMAT, Error> InvalidFormatError;

/**
// An identifier for an enumerated or mask value wasn't able to be converted 
// to an integral value.
// @relates Error
*/
typedef error::ErrorTemplate<PERSIST_ERROR_INVALID_IDENTIFIER, Error> InvalidIdentifierError;

/**
// Not all references in an archive were resolved.
// @relates Error
*/
class SWEET_PERSIST_DECLSPEC UnresolvedReferencesError : public Error
{
    std::multiset<Reference> m_references;

    public:
        UnresolvedReferencesError( std::multiset<Reference>& references, const char* format, ... );
        ~UnresolvedReferencesError() throw ();
        const std::multiset<Reference>& get_references() const;
};

}

}

#endif

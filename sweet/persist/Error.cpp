//
// Error.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "declspec.hpp"
#include "Reference.hpp"
#include "Error.hpp"

using namespace sweet::persist;

/**
// Constructor.
//
// @param error
//  The number that uniquely identifies this Error.
*/
Error::Error( int error )
: sweet::error::Error( error )
{
}

/**
// Constructor.
//
// @param references
//  The References that were left unresolved after reading an Archive.
//
// @param format
//  A printf style format string that describes the error that has occured.
//
// @param ...
//  Arguments as described by \e format.
*/
UnresolvedReferencesError::UnresolvedReferencesError( std::multiset<Reference>& references, const char* format, ... )
: Error( PERSIST_ERROR_UNRESOLVED_REFERENCES ),
  m_references()
{
    m_references.swap( references );

    va_list args;
    va_start( args, format );
    append( format, args );
    va_end( args );
}

/**
// Destructor.
*/
UnresolvedReferencesError::~UnresolvedReferencesError() throw ()
{
}

/**
// Get the references that were left unresolved.
//
// @return
//  The unresolved references.
*/
const std::multiset<Reference>& UnresolvedReferencesError::get_references() const
{
    return m_references;
}

//
// functions.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_FUNCTIONS_HPP_INCLUDED
#define SWEET_PERSIST_FUNCTIONS_HPP_INCLUDED

#include "declspec.hpp"
#include <string>
#include <locale>

namespace sweet
{

namespace persist
{

class Archive;

template <class Type> void* create();
template <class Archive, class Type> void persist( Archive& archive, void* object );

SWEET_PERSIST_DECLSPEC std::string narrow( const std::wstring& wide_string, char unknown = '_', const std::locale& locale = std::locale() );
SWEET_PERSIST_DECLSPEC std::wstring widen( const std::string& narrow_string, const std::locale& locale = std::locale() );
SWEET_PERSIST_DECLSPEC std::wstring widen( const wchar_t* narrow_string, const std::locale& locale = std::locale() );

}

}

#endif

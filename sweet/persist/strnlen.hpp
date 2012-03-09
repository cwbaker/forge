//
// strnlen.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_STRNLEN_HPP_INCLUDED
#define SWEET_PERSIST_STRNLEN_HPP_INCLUDED

#include "declspec.hpp"

namespace sweet
{

namespace persist
{

SWEET_PERSIST_DECLSPEC size_t strnlen( const char* string, size_t max );
SWEET_PERSIST_DECLSPEC size_t strnlen( const wchar_t* string, size_t max );

}

}

#endif

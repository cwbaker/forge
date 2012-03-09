//
// strnlen.hpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_STRNLEN_HPP_INCLUDED
#define SWEET_PERSIST_STRNLEN_HPP_INCLUDED

#include "declspec.hpp"

namespace sweet
{

namespace persist
{

SWEET_PERSIST_DECLSPEC unsigned int strnlen( const char* string, unsigned int max );
SWEET_PERSIST_DECLSPEC unsigned int strnlen( const wchar_t* string, unsigned int max );

}

}

#endif

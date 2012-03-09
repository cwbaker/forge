//
// Error.cpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Error.hpp"

using namespace sweet::process;

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

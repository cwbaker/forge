//
// ForgeEventSink.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ForgeEventSink.hpp"

using namespace sweet::forge;

/**
// Destructor.
*/
ForgeEventSink::~ForgeEventSink()
{
}

/**
// Called when the Forge has received a line of output from a child
// process.
//
// @param forge
//  The Forge that the output has been received from.
//
// @param message
//  The null terminated message to be output.
*/
void ForgeEventSink::forge_output( Forge* /*forge*/, const char* /*message*/ )
{    
}

/**
// Called when the Forge has received a warning to be output.
//
// @param forge
//  The Forge that the warning is generated from.
//
// @param message
//  The null terminated warning message.
*/
void ForgeEventSink::forge_warning( Forge* /*forge*/, const char* /*message*/ )
{
}

/**
// Called when the Forge has received an error to be output.
//
// @param forge
//  The Forge that the error occured in.
//
// @param message
//  The null terminated error message.
*/
void ForgeEventSink::forge_error( Forge* /*forge*/, const char* /*message*/ )
{
}

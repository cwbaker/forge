//
// BuildToolEventSink.cpp
// Copyright (c) 2010 - 2015 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "BuildToolEventSink.hpp"

using namespace sweet::build_tool;

/**
// Destructor.
*/
BuildToolEventSink::~BuildToolEventSink()
{
}

/**
// Called when the BuildTool has received a line of output from a child
// process.
//
// @param build_tool
//  The BuildTool that the output has been received from.
//
// @param message
//  The null terminated message to be output.
*/
void BuildToolEventSink::build_tool_output( BuildTool* /*build_tool*/, const char* /*message*/ )
{    
}

/**
// Called when the BuildTool has received a warning to be output.
//
// @param build_tool
//  The BuildTool that the warning is generated from.
//
// @param message
//  The null terminated warning message.
*/
void BuildToolEventSink::build_tool_warning( BuildTool* /*build_tool*/, const char* /*message*/ )
{
}

/**
// Called when the BuildTool has received an error to be output.
//
// @param build_tool
//  The BuildTool that the error occured in.
//
// @param message
//  The null terminated error message.
*/
void BuildToolEventSink::build_tool_error( BuildTool* /*build_tool*/, const char* /*message*/ )
{
}

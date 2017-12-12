//
// TargetPrototype.cpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "TargetPrototype.hpp"

using namespace sweet::build_tool;

/**
// Constructor.
//
// @param id
//  The id for this TargetPrototype.
//
// @param build_tool
//  The BuildTool that this TargetPrototype is part of.
*/
TargetPrototype::TargetPrototype( const std::string& id, BuildTool* build_tool )
: id_( id ),
  build_tool_( build_tool )  
{
    SWEET_ASSERT( build_tool_ );
}

/**
// Get the id for this TargetPrototype.
//
// @return
//  The id.
*/
const std::string& TargetPrototype::get_id() const
{
    return id_;
}

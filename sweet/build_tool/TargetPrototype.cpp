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
// @param bind_type
//  The BindType for this TargetPrototype.
//
// @param build_tool
//  The BuildTool that this TargetPrototype is part of.
*/
TargetPrototype::TargetPrototype( const std::string& id, BindType bind_type, BuildTool* build_tool )
: id_( id ),
  bind_type_( bind_type ),
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

/**
// Get the BindType for this TargetPrototype.
//
// @return 
//  The BindType.
*/
BindType TargetPrototype::get_bind_type() const
{
    return bind_type_;
}

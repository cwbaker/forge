//
// Rule.cpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Rule.hpp"

using namespace sweet::build_tool;

/**
// Constructor.
//
// @param id
//  The id for this Rule.
//
// @param bind_type
//  The BindType for this Rule.
//
// @param build_tool
//  The BuildTool that this Rule is part of.
*/
Rule::Rule( const std::string& id, BindType bind_type, BuildTool* build_tool )
: id_( id ),
  bind_type_( bind_type ),
  build_tool_( build_tool )  
{
    SWEET_ASSERT( build_tool_ );
}

/**
// Get the id for this Rule.
//
// @return
//  The id.
*/
const std::string& Rule::get_id() const
{
    return id_;
}

/**
// Get the BindType for this Rule.
//
// @return 
//  The BindType.
*/
BindType Rule::get_bind_type() const
{
    return bind_type_;
}

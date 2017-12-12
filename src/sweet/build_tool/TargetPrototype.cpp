//
// TargetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "TargetPrototype.hpp"
#include "BuildTool.hpp"
#include <sweet/assert/assert.hpp>

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
// Destructor.
*/
TargetPrototype::~TargetPrototype()
{
    build_tool_->destroy_target_prototype_lua_binding( this );
}

/**
// Get the id for this TargetPrototype.
//
// @return
//  The id.
*/
const std::string& TargetPrototype::id() const
{
    return id_;
}

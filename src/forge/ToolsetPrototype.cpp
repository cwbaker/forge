//
// ToolsetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ToolsetPrototype.hpp"
#include "Forge.hpp"
#include <assert/assert.hpp>

using namespace sweet::forge;

/**
// Constructor.
//
// @param id
//  The id for this ToolsetPrototype.
//
// @param forge
//  The Forge that this ToolsetPrototype is part of.
*/
ToolsetPrototype::ToolsetPrototype( const std::string& id, Forge* forge )
: id_( id ),
  forge_( forge )  
{
    SWEET_ASSERT( forge_ );
}

/**
// Destructor.
*/
ToolsetPrototype::~ToolsetPrototype()
{
    forge_->destroy_toolset_prototype_lua_binding( this );
}

/**
// Get the id for this ToolsetPrototype.
//
// @return
//  The id.
*/
const std::string& ToolsetPrototype::id() const
{
    return id_;
}

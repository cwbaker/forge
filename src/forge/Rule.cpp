//
// TargetPrototype.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "TargetPrototype.hpp"
#include "Forge.hpp"
#include <assert/assert.hpp>

using namespace sweet::forge;

/**
// Constructor.
//
// @param id
//  The id for this TargetPrototype.
//
// @param forge
//  The Forge that this TargetPrototype is part of.
*/
TargetPrototype::TargetPrototype( const std::string& id, Forge* forge )
: id_( id ),
  forge_( forge )  
{
    SWEET_ASSERT( forge_ );
}

/**
// Destructor.
*/
TargetPrototype::~TargetPrototype()
{
    forge_->destroy_target_prototype_lua_binding( this );
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

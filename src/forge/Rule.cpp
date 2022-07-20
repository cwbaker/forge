//
// Rule.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Rule.hpp"
#include "Forge.hpp"
#include <assert/assert.hpp>

using namespace sweet::forge;

/**
// Constructor.
//
// @param id
//  The id for this Rule.
//
// @param forge
//  The Forge that this Rule is part of.
*/
Rule::Rule( const std::string& id, Forge* forge )
: id_( id )
, forge_( forge )  
{
    SWEET_ASSERT( forge_ );
}

/**
// Destructor.
*/
Rule::~Rule()
{
    forge_->destroy_rule_lua_binding( this );
}

/**
// Get the id for this Rule.
//
// @return
//  The id.
*/
const std::string& Rule::id() const
{
    return id_;
}

//
// Toolset.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "Toolset.hpp"
#include "Graph.hpp"
#include "Forge.hpp"
#include <assert/assert.hpp>

using namespace sweet;
using namespace sweet::forge;

Toolset::Toolset( const std::string& id, Graph* graph )
: id_( id )
, graph_( graph )
{
    SWEET_ASSERT( graph_ );
}

Toolset::~Toolset()
{
    if ( graph_ )
    {
        graph_->forge()->destroy_toolset_lua_binding( this );
    }
}

const std::string& Toolset::id() const
{
    return id_;
}

Graph* Toolset::graph() const
{
    return graph_;
}

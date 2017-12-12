#ifndef SWEET_BUILD_TOOL_PERSIST_HPP_INCLUDED
#define SWEET_BUILD_TOOL_PERSIST_HPP_INCLUDED

#include "Graph.hpp"
#include "Error.hpp"
#include <sweet/rtti/macros.hpp>
#include <sweet/persist/vector.hpp>
#include <sweet/persist/persist.hpp>

namespace sweet
{

namespace build_tool
{

/**
// Declare the types persisted in an Archive.
//
// @param archive
//  The Archive to declare the types with.
*/
template <class Archive> void Graph::enter( Archive& archive )
{
    using namespace sweet::persist;
    SWEET_ASSERT( build_tool_ );
    archive.set_context( SWEET_STATIC_TYPEID(BuildTool), build_tool_ );
    archive.template declare<Graph>( "Graph", PERSIST_NORMAL );
    archive.template declare<Target>( "Target", PERSIST_NORMAL );
}

/**
// Destroy state created for persisting this Graph.
//
// @param archive
//  The Archive that state was created for.
*/
template <class Archive> void Graph::exit( Archive& /*archive*/ )
{
}

/**
// Persist this Graph with an Archive.
//
// @param archive
//  The Graph to persist this Archive with.
*/
template <class Archive> void Graph::persist( Archive& archive )
{
    const int BUILD_GRAPH_VERSION = 29;
    archive.enter( "Sweet Build Graph", BUILD_GRAPH_VERSION, *this );
    if ( archive.version() == BUILD_GRAPH_VERSION )
    {
        archive.value( "root", root_target_ );        
    }
}

/**
// Persist this Target with an Archive.
//
// @param archive
//  The Archive to persist this Target with.
//
// @return
//  Nothing.
*/
template <class Archive> void Target::persist( Archive& archive )
{
    archive.value( "id", id_ );
    archive.value( "last_write_time", last_write_time_ );
    archive.value( "built", built_ );
    archive.value( "filenames", filenames_ );
    archive.value( "targets", "target", targets_ );
    archive.refer( "implicit_dependencies", "dependency", implicit_dependencies_ );
}

}

}

#endif
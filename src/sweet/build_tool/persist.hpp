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

/**
// Save a TargetPrototype to an Archive.
//
// @param archive
//  The Archive to save the TargetPrototype in.
//
// @param mode
//  The Mode to use when saving (must be MODE_VALUE).
//
// @param name
//  The name of the attribute to save the TargetPrototype in.
//
// @param target_prototype
//  The TargetPrototype to save.
*/
template <class Archive> void save( Archive& archive, int mode, const char* name, TargetPrototype*& target_prototype )
{
    SWEET_ASSERT( mode == sweet::persist::MODE_VALUE );
    (void) mode;

    if ( target_prototype )
    {
        std::string id = target_prototype->id();
        archive.value( name, id );
    }
    else
    {
        std::string id;
        archive.value( name, id );
    }
}

/**
// Load a TargetPrototype from an Archive.
//
// @param archive
//  The Archive to load the TargetPrototype from.
//
// @param mode
//  The Mode to use when loading (must be MODE_VALUE).
//
// @param name
//  The name of the attribute to load the TargetPrototype from.
//
// @param target_prototype
//  The TargetPrototype to load into (assumed to be null).
*/
template <class Archive> void load( Archive& archive, int mode, const char* name, TargetPrototype*& target_prototype )
{
    SWEET_ASSERT( mode == sweet::persist::MODE_VALUE );
    SWEET_ASSERT( !target_prototype );
    (void) mode;

    std::string id;
    archive.value( name, id );

    if ( !id.empty() )
    {
        BuildTool* build_tool = reinterpret_cast<BuildTool*>( archive.get_context(SWEET_STATIC_TYPEID(BuildTool)) );
        SWEET_ASSERT( build_tool );
        target_prototype = build_tool->graph()->target_prototype( id );
    }
}

/**
// Resolve a TargetPrototype that has been loaded from an Archive.
//
// @param archive
//  The Archive to resolve the TargetPrototype from (ignored).
//
// @param mode
//  The Mode to use when resolving (ignored).
//
// @param target_prototype
//  The TargetPrototype to load into (ignored).
*/
template <class Archive> void resolve( Archive& /*archive*/, int /*mode*/, TargetPrototype*& /*target_prototype*/ )
{
}

}

}

#endif
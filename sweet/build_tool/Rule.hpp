//
// BuildTool.hpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_TARGETPROTOTYPE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_TARGETPROTOTYPE_HPP_INCLUDED

#include "declspec.hpp"
#include "BindType.hpp"
#include <sweet/pointer/ptr.hpp>
#include <string>

namespace sweet
{

namespace build_tool
{

class BuildTool;

/**
// A prototype for Targets.
*/
class SWEET_BUILD_TOOL_DECLSPEC Rule
{
    std::string id_; ///< The identifier for this Rule.
    BindType bind_type_; ///< How Targets for this Rule are associated with files.      
    BuildTool* build_tool_; ///< The BuildTool that this Rule is part of.

    public:
        Rule( const std::string& id, BindType bind_type, BuildTool* build_tool );
        const std::string& get_id() const;
        BindType get_bind_type() const;
};

/**
// Save a Rule to an Archive.
//
// @param archive
//  The Archive to save the Rule in.
//
// @param mode
//  The Mode to use when saving (must be MODE_VALUE).
//
// @param name
//  The name of the attribute to save the Rule in.
//
// @param rule
//  The Rule to save.
*/
template <class Archive> 
void save( Archive& archive, int mode, const char* name, ptr<Rule>& rule )
{
    SWEET_ASSERT( mode == sweet::persist::MODE_VALUE );

    if ( rule )
    {
        std::string id = rule->get_id();
        archive.value( name, id );
    }
    else
    {
        std::string id;
        archive.value( name, id );
    }
}

/**
// Load a Rule from an Archive.
//
// @param archive
//  The Archive to load the Rule from.
//
// @param mode
//  The Mode to use when loading (must be MODE_VALUE).
//
// @param name
//  The name of the attribute to load the Rule from.
//
// @param rule
//  The Rule to load into (assumed to be null).
*/
template <class Archive> 
void load( Archive& archive, int mode, const char* name, ptr<Rule>& rule )
{
    SWEET_ASSERT( mode == sweet::persist::MODE_VALUE );
    SWEET_ASSERT( !rule );

    std::string id;
    archive.value( name, id );

    if ( !id.empty() )
    {
        BuildTool* build_tool = reinterpret_cast<BuildTool*>( archive.get_context(SWEET_STATIC_TYPEID(BuildTool)) );
        SWEET_ASSERT( build_tool );
        rule = build_tool->get_script_interface()->rule( id, BIND_PHONY );
    }
}

/**
// Resolve a Rule that has been loaded from an Archive.
//
// @param archive
//  The Archive to resolve the Rule from (ignored).
//
// @param mode
//  The Mode to use when resolving (ignored).
//
// @param rule
//  The Rule to load into (ignored).
*/
template <class Archive>
void resolve( Archive& archive, int mode, ptr<Rule>& rule )
{
}

}

}

#endif

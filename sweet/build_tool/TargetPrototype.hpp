//
// BuildTool.hpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_TARGET_PROTOTYPE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_TARGET_PROTOTYPE_HPP_INCLUDED

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
class SWEET_BUILD_TOOL_DECLSPEC TargetPrototype
{
    std::string id_; ///< The identifier for this TargetPrototype.
    BindType bind_type_; ///< How Targets for this TargetPrototype are associated with files.      
    BuildTool* build_tool_; ///< The BuildTool that this TargetPrototype is part of.

    public:
        TargetPrototype( const std::string& id, BindType bind_type, BuildTool* build_tool );
        const std::string& get_id() const;
        BindType get_bind_type() const;
};

}

}

#endif

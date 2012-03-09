//
// BindType.hpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_BUILD_TOOL_BINDTYPE_HPP_INCLUDED
#define SWEET_BUILD_TOOL_BINDTYPE_HPP_INCLUDED

namespace sweet
{

namespace build_tool
{

/**
// How Targets are bound to files, how they are considered outdated, and how
// their timestamp is calculated from their dependencies.
*/
enum BindType
{
    BIND_NULL, ///< Targets take their bind type from their Rule's bind type.
    BIND_PHONY, ///< Targets are not associated with a file.
    BIND_DIRECTORY, ///< Targets are associated with directories.
    BIND_SOURCE_FILE, ///< Targets are associated with a source file.
    BIND_INTERMEDIATE_FILE, ///< Targets are associated with an intermediate file.
    BIND_GENERATED_FILE, ///< Targets are associated with a generated file.
    BIND_TYPE_COUNT
};

}

}

#endif
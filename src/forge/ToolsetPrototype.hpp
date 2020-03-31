#ifndef FORGE_TOOLSETPROTOTYPE_HPP_INCLUDED
#define FORGE_TOOLSETPROTOTYPE_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace forge
{

class Forge;

/**
// A prototype for Toolsets.
*/
class ToolsetPrototype
{
    std::string id_; ///< The identifier for this ToolsetPrototype.
    Forge* forge_; ///< The Forge that this ToolsetPrototype is part of.

    public:
        ToolsetPrototype( const std::string& id, Forge* forge );
        ~ToolsetPrototype();
        const std::string& id() const;
};

}

}

#endif

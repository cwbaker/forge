#ifndef FORGE_TARGET_PROTOTYPE_HPP_INCLUDED
#define FORGE_TARGET_PROTOTYPE_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace forge
{

class Forge;

/**
// A prototype for Targets.
*/
class TargetPrototype
{
    std::string id_; ///< The identifier for this TargetPrototype.
    Forge* forge_; ///< The Forge that this TargetPrototype is part of.

    public:
        TargetPrototype( const std::string& id, Forge* forge );
        ~TargetPrototype();
        const std::string& id() const;
};

}

}

#endif

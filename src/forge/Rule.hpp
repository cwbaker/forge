#pragma once

#include <string>

namespace sweet
{

namespace forge
{

class Forge;

/**
// A prototype for Targets.
*/
class Rule
{
    std::string id_; ///< The identifier for this rule.
    Forge* forge_; ///< The Forge that this rule is part of.

public:
    Rule( const std::string& id, Forge* forge );
    ~Rule();
    const std::string& id() const;
};

}

}

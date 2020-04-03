#ifndef FORGE_TOOLSET_HPP_INCLUDED
#define FORGE_TOOLSET_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace forge
{

class ToolsetPrototype;
class Graph;

/**
// A toolset.
*/
class Toolset
{
    std::string id_; ///< The unique identifier for this Toolset.
    Graph* graph_; ///< The Graph that this Toolset is part of.
    ToolsetPrototype* prototype_; ///< The ToolsetPrototype for this Toolset or null if this Toolset has no ToolsetPrototype.

    public:
        Toolset( const std::string& id, ToolsetPrototype* toolset_prototype, Graph* graph );
        ~Toolset();
        const std::string& id() const;
        Graph* graph() const;
        ToolsetPrototype* prototype() const;
};

}

}

#endif

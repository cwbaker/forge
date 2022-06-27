#ifndef FORGE_TOOLSET_HPP_INCLUDED
#define FORGE_TOOLSET_HPP_INCLUDED

#include <string>

namespace sweet
{

namespace forge
{

class Graph;

/**
// A toolset.
*/
class Toolset
{
    std::string id_; ///< The unique identifier for this Toolset.
    Graph* graph_; ///< The Graph that this Toolset is part of.

    public:
        Toolset( const std::string& id, Graph* graph );
        ~Toolset();
        const std::string& id() const;
        Graph* graph() const;
};

}

}

#endif

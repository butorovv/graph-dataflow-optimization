#ifndef IPATHFINDER_H
#define IPATHFINDER_H

#include "../domain/NetworkTypes.h"
#include "../domain/NetworkGraph.h"
#include <memory>

namespace Infrastructure
{
    class IPathFinder
    {
    public:
        virtual ~IPathFinder() = default;
        virtual Domain::PathResult findShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id) = 0;
        virtual std::string getAlgorithmName() const = 0;
    };
    using IPathFinderPtr = std::unique_ptr<IPathFinder>;
}

#endif
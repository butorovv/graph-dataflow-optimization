// include/infrastructure/BGLShortestPath.h
#ifndef BGLSHORTESTPATH_H
#define BGLSHORTESTPATH_H
#include "../domain/NetworkGraph.h"
#include "../domain/NetworkTypes.h"
#include "IPathFinder.h"

namespace Infrastructure
{
    class BGLShortestPath : public IPathFinder
    {
    public:
        BGLShortestPath() = default;
        ~BGLShortestPath() override = default;

        Domain::PathResult findShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id) override;
        static Domain::PathResult findShortestPathStatic(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id);
    };

}

#endif
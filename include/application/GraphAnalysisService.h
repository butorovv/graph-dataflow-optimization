#ifndef GRAPHANALYSISSERVICE_H
#define GRAPHANALYSISSERVICE_H

#include <vector>
#include <memory>
#include <utility>

namespace Domain
{
    class NetworkGraph;
    using NetworkGraphPtr = std::shared_ptr<NetworkGraph>;

    struct PathResult;
    class IGraphRepository;
}
namespace Infrastructure
{
    class IPathFinder;
    using IPathFinderPtr = std::unique_ptr<IPathFinder>;
}

namespace Application
{
    class GraphAnalysisService
    {
    public:
        GraphAnalysisService() = default;
        Domain::PathResult analyzeShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id);

        std::vector<Domain::PathResult> analyzePathsParallel(
            const Domain::NetworkGraphPtr &graph,
            const std::vector<std::pair<int, int>> &routes);
    };

}

#endif
#include "application/GraphAnalysisService.h"
#include "application/GraphAnalysisFactory.h"
#include "infrastructure/ThreadPool.h"
#include <chrono>

namespace Application
{

    Domain::PathResult GraphAnalysisService::analyzeShortestPath(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id)
    {
        auto finder = GraphAnalysisFactory::createPathFinder();
        return finder->findShortestPath(graph, start_id, end_id);
    }

    std::vector<Domain::PathResult> GraphAnalysisService::analyzePathsParallel(
        const Domain::NetworkGraphPtr &graph,
        const std::vector<std::pair<int, int>> &routes)
    {
        Infrastructure::ThreadPool pool(4);
        std::vector<std::future<Domain::PathResult>> futures;

        for (const auto &[start, end] : routes)
        {
            futures.push_back(pool.enqueue(
                [graph, start, end]()
                {
                    auto finder = Application::GraphAnalysisFactory::createPathFinder();
                    return finder->findShortestPath(graph, start, end);
                }));
        }

        std::vector<Domain::PathResult> results;
        for (auto &fut : futures)
        {
            results.push_back(fut.get());
        }
        return results;
    }

}
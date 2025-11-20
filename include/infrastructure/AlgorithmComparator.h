#ifndef ALGORITHMCOMPARATOR_H
#define ALGORITHMCOMPARATOR_H

#include "../domain/NetworkGraph.h"
#include "BGLShortestPath.h"
#include "AStarPathFinder.h"
#include "GeneticAlgorithm.h"
#include <vector>
#include <map>

namespace Infrastructure
{
    struct AlgorithmComparison {
        std::string algorithmName;
        double executionTime;
        double pathCost;
        int pathLength;
        bool success;
    };

    class AlgorithmComparator
    {
    public:
        static std::vector<AlgorithmComparison> compareAlgorithms(
            const Domain::NetworkGraphPtr &graph,
            const std::vector<std::pair<int, int>> &test_routes);
        
        static void printComparisonTable(const std::vector<AlgorithmComparison> &results);
    };
}

#endif
#ifndef ALGORITHMCOMPARATOR_H
#define ALGORITHMCOMPARATOR_H

#include "../domain/NetworkGraph.h"
#include "BGLShortestPath.h"
#include "AStarPathFinder.h"
#include "GeneticAlgorithm.h"
#include "AntColonyOptimizer.h"
#include <vector>
#include <map>
#include <memory>

namespace Infrastructure
{
    struct AlgorithmComparison {
        std::string algorithmName;
        double executionTime;
        double pathCost;
        int pathLength;
        bool success;
        std::string algorithmType; // exact and heuristic
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
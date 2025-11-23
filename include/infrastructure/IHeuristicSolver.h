#ifndef IHEURISTICSOLVER_H
#define IHEURISTICSOLVER_H

#include "domain/NetworkGraph.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace Infrastructure
{
    struct HeuristicResult
    {
        bool success = false;
        double objective = 0.0;
        std::vector<std::pair<int, int>> path;
        std::map<std::pair<int, int>, double> flows;
        double executionTime = 0.0;
        std::string algorithmName;
    };

    class IHeuristicSolver
    {
    public:
        virtual ~IHeuristicSolver() = default;
        virtual HeuristicResult optimize(const Domain::NetworkGraphPtr &graph,
                                         const std::vector<std::pair<int, int>> &demands) = 0;
    };

    using IHeuristicSolverPtr = std::unique_ptr<IHeuristicSolver>;
}

#endif
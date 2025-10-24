#ifndef BOOSTFLOWSOLVER_H
#define BOOSTFLOWSOLVER_H

#include "IFlowSolver.h"
#include "../domain/NetworkGraph.h"
#include <unordered_map>
#include <utility>

// хэш-функция для std::pair
namespace std
{
    template <>
    struct hash<std::pair<int, int>>
    {
        size_t operator()(const std::pair<int, int> &p) const
        {
            return hash<int>()(p.first) ^ (hash<int>()(p.second) << 1);
        }
    };
}

namespace Infrastructure
{
    class BoostFlowSolver : public IFlowSolver
    {
    public:
        FlowResult solveMaxFlow(const Domain::NetworkGraphPtr &graph, int source, int sink) override;
        FlowResult solveMinCostMaxFlow(const Domain::NetworkGraphPtr &graph, int source, int sink) override;
    };
}

#endif

#ifndef IFLOWSOLVER_H
#define IFLOWSOLVER_H

#include "../domain/NetworkGraph.h"
#include <map>
#include <string>
#include <memory>

namespace Infrastructure
{
    struct FlowResult
    {
        double maxFlow = 0.0;                              // объем максимального потока
        double totalCost = 0.0;                            // время выполнения алгоритма (мс)
        double flowCost = 0.0;                             // суммарная стоимость потока (только для MinCostFlow)
        std::map<std::pair<int, int>, double> flowPerEdge; // поток по каждому ребру
        std::string algorithmName;
        bool success = false;
        std::string errorMessage;
    };

    class IFlowSolver
    {
    public:
        virtual ~IFlowSolver() = default;
        virtual FlowResult solveMaxFlow(const Domain::NetworkGraphPtr &graph,
                                        int source, int sink) = 0;
        virtual FlowResult solveMinCostMaxFlow(const Domain::NetworkGraphPtr &graph,
                                               int source, int sink) = 0;
    };

    using IFlowSolverPtr = std::unique_ptr<IFlowSolver>;

}

#endif

#ifndef BGLSHORTESTPATH_H
#define BGLSHORTESTPATH_H

#include "../domain/NetworkGraph.h"
#include "../domain/NetworkTypes.h"
#include "../domain/WeightCalculator.h"
#include "IPathFinder.h"
#include <chrono>

namespace Infrastructure
{
    class BGLShortestPath : public IPathFinder
    {
    public:
        BGLShortestPath(bool useWeights = true, 
                       Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY)
            : useWeights(useWeights), strategy(strategy) {}
        
        ~BGLShortestPath() override = default;

        Domain::PathResult findShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id) override;

        std::string getAlgorithmName() const override {
            if (!useWeights) {
                return "BGL Dijkstra (Uniform)";
            }
            return "BGL Dijkstra (" + Domain::WeightCalculator::getStrategyName(strategy) + ")";
        }

        // статический метод с параметрами
        static Domain::PathResult findShortestPathStatic(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id,
            bool useWeights = true,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY);

    private:
        bool useWeights;
        Domain::WeightCalculator::Strategy strategy;
    };
}

#endif
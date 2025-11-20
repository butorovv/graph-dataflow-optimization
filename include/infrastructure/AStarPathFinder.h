#ifndef ASTARPATHFINDER_H
#define ASTARPATHFINDER_H

#include "IPathFinder.h"
#include "../domain/WeightCalculator.h"
#include <unordered_map>
#include <queue>
#include <vector>
#include <functional>
#include <chrono>

namespace Infrastructure
{
    class AStarPathFinder : public IPathFinder
    {
    public:
        AStarPathFinder(bool useWeights = true,
                       Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY)
            : useWeights(useWeights), strategy(strategy) {}

        Domain::PathResult findShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id) override;

    private:
        bool useWeights;
        Domain::WeightCalculator::Strategy strategy;
        
        // эвристическая функция (можно расширить для разных эвристик)
        double heuristic(int current, int target) const {
            // нулевая эвристика = дейкстра
            // в будущем можно добавить эвристику на основе координат или других метрик
            (void)current; (void)target;
            return 0.0;
        }
        
        struct Node {
            int id;
            double f_score;
            double g_score;
            
            bool operator>(const Node& other) const {
                return f_score > other.f_score;
            }
        };
    };
}

#endif
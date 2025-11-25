#ifndef ASTARPATHFINDER_H
#define ASTARPATHFINDER_H

#include "IPathFinder.h"
#include "../domain/WeightCalculator.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>
#include <vector>
#include <unordered_map>

namespace Infrastructure
{
    struct found_goal {};

    using BGLGraph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::directedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double>>;

    template <class Graph, class CostType = double>
    class AStarHeuristic : public boost::astar_heuristic<Graph, CostType>
    {
    public:
        using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
        explicit AStarHeuristic(Vertex goal) : m_goal(goal) {}
        CostType operator()(Vertex) const { return 0; } // нулевая эвристика
    private:
        Vertex m_goal;
    };

    template <class Vertex>
    class AStarGoalVisitor : public boost::default_astar_visitor
    {
    public:
        explicit AStarGoalVisitor(Vertex goal) : m_goal(goal) {}
        template <class Graph>
        void examine_vertex(Vertex u, const Graph&)
        {
            if (u == m_goal)
                throw found_goal();
        }
    private:
        Vertex m_goal;
    };

    class AStarPathFinder : public IPathFinder
    {
    public:
        AStarPathFinder(bool useWeights = true,
                        Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY)
            : useWeights(useWeights), strategy(strategy), useCoordinateHeuristic(false) {}

        Domain::PathResult findShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id) override;

        static Domain::PathResult findShortestPathStatic(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id,
            bool useWeights = true,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY,
            bool useCoordinateHeuristic = false);

        void enableCoordinateHeuristic(bool enable) { useCoordinateHeuristic = enable; }

        std::string getAlgorithmName() const override
        {
            if (!useWeights)
                return "BGL A* (Uniform)";
            return "BGL A* (" + Domain::WeightCalculator::getStrategyName(strategy) + ")";
        }

    private:
        bool useWeights;
        Domain::WeightCalculator::Strategy strategy;
        bool useCoordinateHeuristic;
    };
}

#endif

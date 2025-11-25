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

    // BGL граф для A*
    using BGLGraph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::directedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double>>;

    // эвристическая функция для A*
    template <class Graph, class CostType>
    class AStarHeuristic : public boost::astar_heuristic<Graph, CostType>
    {
    public:
        using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
        
        AStarHeuristic(Vertex goal) : m_goal(goal) {}
        
        CostType operator()(Vertex u) const {
            // нулевая эвристика для честного сравнения с Dijkstra
            return 0;
        }
        
    private:
        Vertex m_goal;
    };

    // visitor для обнаружения цели
    template <class Vertex>
    class AStarGoalVisitor : public boost::default_astar_visitor
    {
    public:
        AStarGoalVisitor(Vertex goal) : m_goal(goal) {}
        
        template <class Graph>
        void examine_vertex(Vertex u, const Graph& g) {
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
            : useWeights(useWeights), strategy(strategy) {}

        Domain::PathResult findShortestPath(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id) override;
            
        // статическая версия для использования в компараторе
        static Domain::PathResult findShortestPathStatic(
            const Domain::NetworkGraphPtr &graph,
            int start_id, int end_id,
            bool useWeights = true,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY);

        std::string getAlgorithmName() const override {
            if (!useWeights) {
                return "BGL A* (Uniform)";
            }
            return "BGL A* (" + Domain::WeightCalculator::getStrategyName(strategy) + ")";
        }

    private:
        bool useWeights;
        Domain::WeightCalculator::Strategy strategy;
    };
}

#endif
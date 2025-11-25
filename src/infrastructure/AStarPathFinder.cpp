#include "infrastructure/AStarPathFinder.h"
#include <limits>
#include <algorithm>
#include <cmath>
#include <chrono>

namespace Infrastructure
{
    struct CoordNode
    {
        int id;
        double x, y;
        CoordNode(int nid = 0, double nx = 0, double ny = 0) : id(nid), x(nx), y(ny) {}
    };

    template <typename Graph>
    struct CoordHeuristic
    {
        const std::vector<CoordNode>& nodes;
        const std::unordered_map<int, size_t>& node_to_index;
        size_t goal_index;

        CoordHeuristic(const std::vector<CoordNode>& n,
                       const std::unordered_map<int, size_t>& map,
                       int goal_id)
            : nodes(n), node_to_index(map), goal_index(map.at(goal_id)) {}

        double operator()(size_t u) const
        {
            double dx = nodes[u].x - nodes[goal_index].x;
            double dy = nodes[u].y - nodes[goal_index].y;
            return std::sqrt(dx*dx + dy*dy);
        }
    };

    Domain::PathResult AStarPathFinder::findShortestPath(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id)
    {
        return findShortestPathStatic(graph, start_id, end_id,
                                      useWeights, strategy, useCoordinateHeuristic);
    }

    Domain::PathResult AStarPathFinder::findShortestPathStatic(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id,
        bool useWeights,
        Domain::WeightCalculator::Strategy strategy,
        bool useCoordinateHeuristic)
    {
        const int RUNS = 100;
        auto total_duration = std::chrono::nanoseconds::zero();
        Domain::PathResult result;

        if (!graph->hasNode(start_id) || !graph->hasNode(end_id))
        {
            result.success = false;
            result.errorMessage = "Start or end node not found";
            return result;
        }

        auto node_ids = graph->getAllNodeIds();
        BGLGraph bgl_graph(node_ids.size());
        std::unordered_map<int, size_t> node_to_index;
        std::vector<int> index_to_node;
        index_to_node.reserve(node_ids.size());

        std::vector<CoordNode> coord_nodes;
        coord_nodes.reserve(node_ids.size());

        for (size_t i = 0; i < node_ids.size(); ++i)
        {
            int nid = node_ids[i];
            node_to_index[nid] = i;
            index_to_node.push_back(nid);

            double x = static_cast<double>(nid);
            double y = (nid >= 1000) ? 10000.0 + static_cast<double>(nid % 1000) : 0.0;
            coord_nodes.emplace_back(nid, x, y);
        }

        for (int node_id : node_ids)
        {
            size_t u = node_to_index[node_id];
            for (int v_id : graph->getNeighbors(node_id))
            {
                double weight = 1.0;
                if (useWeights)
                {
                    try { weight = graph->getEdgeWeight(node_id, v_id, strategy); }
                    catch (...) { weight = 1.0; }
                }
                boost::add_edge(u, node_to_index[v_id], weight, bgl_graph);
            }
        }

        size_t n = index_to_node.size();
        size_t start_idx = node_to_index[start_id];
        size_t end_idx = node_to_index[end_id];

        std::vector<double> distances(n, std::numeric_limits<double>::infinity());
        std::vector<BGLGraph::vertex_descriptor> predecessors(n);
        for (size_t i = 0; i < n; ++i)
            predecessors[i] = i;

        try
        {
            if (useCoordinateHeuristic)
            {
                CoordHeuristic<BGLGraph> heuristic(coord_nodes, node_to_index, end_id);
                auto visitor = AStarGoalVisitor<BGLGraph::vertex_descriptor>(end_idx);
                boost::astar_search(bgl_graph, start_idx, heuristic,
                                    boost::predecessor_map(&predecessors[0])
                                        .distance_map(&distances[0])
                                        .visitor(visitor));
            }
            else
            {
                AStarHeuristic<BGLGraph, double> heuristic(end_idx);
                auto visitor = AStarGoalVisitor<BGLGraph::vertex_descriptor>(end_idx);
                boost::astar_search(bgl_graph, start_idx, heuristic,
                                    boost::predecessor_map(&predecessors[0])
                                        .distance_map(&distances[0])
                                        .visitor(visitor));
            }
        }
        catch (const found_goal&) {}
        catch (const std::exception& e)
        {
            result.success = false;
            result.errorMessage = "BGL A* error: " + std::string(e.what());
            return result;
        }

        if (distances[end_idx] == std::numeric_limits<double>::infinity())
        {
            result.success = false;
            result.errorMessage = "No path found";
            return result;
        }

        std::vector<int> rev_path;
        for (size_t v = end_idx; v != start_idx; v = predecessors[v])
        {
            if (predecessors[v] == v)
            {
                result.success = false;
                result.errorMessage = "Path reconstruction failed";
                return result;
            }
            rev_path.push_back(index_to_node[v]);
        }
        rev_path.push_back(index_to_node[start_idx]);
        std::reverse(rev_path.begin(), rev_path.end());

        result.success = true;
        result.pathNodes = std::move(rev_path);
        result.totalCost = distances[end_idx];
        result.algorithmName = useWeights ? "BGL A* (Multi-Param + Coord)" : "BGL A* (Uniform + Coord)";

        for (int run = 0; run < RUNS; ++run)
        {
            auto startTime = std::chrono::high_resolution_clock::now();

            std::vector<double> temp_distances(n, std::numeric_limits<double>::infinity());
            std::vector<BGLGraph::vertex_descriptor> temp_predecessors(n);
            for (size_t i = 0; i < n; ++i)
                temp_predecessors[i] = i;

            try
            {
                if (useCoordinateHeuristic)
                {
                    CoordHeuristic<BGLGraph> heuristic(coord_nodes, node_to_index, end_id);
                    auto visitor = AStarGoalVisitor<BGLGraph::vertex_descriptor>(end_idx);
                    boost::astar_search(bgl_graph, start_idx, heuristic,
                                        boost::predecessor_map(&temp_predecessors[0])
                                            .distance_map(&temp_distances[0])
                                            .visitor(visitor));
                }
                else
                {
                    AStarHeuristic<BGLGraph, double> heuristic(end_idx);
                    auto visitor = AStarGoalVisitor<BGLGraph::vertex_descriptor>(end_idx);
                    boost::astar_search(bgl_graph, start_idx, heuristic,
                                        boost::predecessor_map(&temp_predecessors[0])
                                            .distance_map(&temp_distances[0])
                                            .visitor(visitor));
                }
            }
            catch (...) {}

            auto endTime = std::chrono::high_resolution_clock::now();
            total_duration += (endTime - startTime);
        }

        result.executionTime = (total_duration.count() / RUNS) / 1000000.0;
        return result;
    }
}

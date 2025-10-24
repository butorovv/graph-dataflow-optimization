#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include "infrastructure/BGLShortestPath.h"
#include <unordered_map>
#include <vector>
#include <limits>
#include <algorithm>

namespace Infrastructure
{

    using BGLGraph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::directedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double>>;

    Domain::PathResult BGLShortestPath::findShortestPath(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id)
    {
        return findShortestPathStatic(graph, start_id, end_id);
    }

    Domain::PathResult BGLShortestPath::findShortestPathStatic(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id)
    {
        Domain::PathResult result;

        if (!graph->hasNode(start_id) || !graph->hasNode(end_id))
        {
            result.success = false;
            result.errorMessage = "Start or end node not found";
            return result;
        }

        BGLGraph bgl_graph;
        std::unordered_map<int, size_t> node_to_index;
        std::vector<int> index_to_node;

        auto node_ids = graph->getAllNodeIds();
        for (size_t i = 0; i < node_ids.size(); ++i)
        {
            node_to_index[node_ids[i]] = i;
            index_to_node.push_back(node_ids[i]);
            boost::add_vertex(bgl_graph);
        }

        for (int node_id : node_ids)
        {
            size_t u = node_to_index[node_id];
            auto neighbors = graph->getNeighbors(node_id);
            for (int v_id : neighbors)
            {
                double w = graph->getEdgeWeight(node_id, v_id);
                size_t v = node_to_index[v_id];
                boost::add_edge(u, v, w, bgl_graph);
            }
        }
        const size_t n = index_to_node.size();
        std::vector<double> distances(n, std::numeric_limits<double>::infinity());
        std::vector<size_t> predecessors(n, std::numeric_limits<size_t>::max());

        size_t start_idx = node_to_index[start_id];
        size_t end_idx = node_to_index[end_id];

        boost::dijkstra_shortest_paths(
            bgl_graph,
            start_idx,
            boost::predecessor_map(&predecessors[0]).distance_map(&distances[0]));

        if (distances[end_idx] == std::numeric_limits<double>::infinity())
        {
            result.success = false;
            result.errorMessage = "No path found";
            return result;
        }

        // реконструкция пути
        std::vector<int> rev_path;
        for (size_t v = end_idx; v != start_idx; v = predecessors[v])
        {
            if (predecessors[v] == std::numeric_limits<size_t>::max())
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
        result.algorithmName = "BGL Dijkstra";
        return result;
    }

}
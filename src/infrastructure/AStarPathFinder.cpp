#include "infrastructure/AStarPathFinder.h"
#include <limits>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace Infrastructure
{
    Domain::PathResult AStarPathFinder::findShortestPath(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id)
    {
        return findShortestPathStatic(graph, start_id, end_id, useWeights, strategy);
    }

    Domain::PathResult AStarPathFinder::findShortestPathStatic(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id,
        bool useWeights,
        Domain::WeightCalculator::Strategy strategy)
    {
        const int RUNS = 100;  // 100 запусков для усреднения
        auto total_duration = std::chrono::nanoseconds::zero();
        Domain::PathResult result;

        // проверка входных данных
        if (!graph->hasNode(start_id) || !graph->hasNode(end_id))
        {
            result.success = false;
            result.errorMessage = "Start or end node not found";
            return result;
        }

        // ограничение на размер графа
        const size_t MAX_NODES = 5000;
        auto node_ids = graph->getAllNodeIds();
        if (node_ids.size() > MAX_NODES)
        {
            result.success = false;
            result.errorMessage = "Graph too large for BGL A* (" + std::to_string(node_ids.size()) + " nodes)";
            return result;
        }

        // подготовка BGL графа
        BGLGraph bgl_graph;
        std::unordered_map<int, size_t> node_to_index;
        std::vector<int> index_to_node;
        index_to_node.reserve(node_ids.size());

        for (size_t i = 0; i < node_ids.size(); ++i)
        {
            node_to_index[node_ids[i]] = i;
            index_to_node.push_back(node_ids[i]);
        }
        bgl_graph = BGLGraph(node_ids.size());

        // построение графа
        for (int node_id : node_ids)
        {
            size_t u = node_to_index[node_id];
            const auto &neighbors = graph->getNeighbors(node_id);
            for (int v_id : neighbors)
            {
                double weight = 1.0;
                if (useWeights)
                {
                    try
                    {
                        weight = graph->getEdgeWeight(node_id, v_id, strategy);
                    }
                    catch (...)
                    {
                        weight = 1.0;
                    }
                }
                size_t v = node_to_index[v_id];
                boost::add_edge(u, v, weight, bgl_graph);
            }
        }

        const size_t n = index_to_node.size();
        size_t start_idx = node_to_index[start_id];
        size_t end_idx = node_to_index[end_id];

        // один запуск для получения результата пути
        std::vector<double> distances(n, std::numeric_limits<double>::infinity());
        std::vector<BGLGraph::vertex_descriptor> predecessors(n);

        try
        {
            // инициализация предшественников
            for (size_t i = 0; i < n; ++i)
                predecessors[i] = i;

            // запуск алгоритма A* из Boost
            auto heuristic = AStarHeuristic<BGLGraph, double>(end_idx);
            auto visitor = AStarGoalVisitor<BGLGraph::vertex_descriptor>(end_idx);

            boost::astar_search(
                bgl_graph,
                start_idx,
                heuristic,
                boost::predecessor_map(&predecessors[0])
                   .distance_map(&distances[0])
                   .visitor(visitor)
            );

        }
        catch (const found_goal&)
        {
            // путь найден - продолжаем обработку
        }
        catch (const std::exception& e)
        {
            result.success = false;
            result.errorMessage = std::string("BGL A* error: ") + e.what();
            return result;
        }

        // проверка, найден ли путь
        if (distances[end_idx] == std::numeric_limits<double>::infinity())
        {
            result.success = false;
            result.errorMessage = "No path found";
            return result;
        }

        // восстановление пути
        std::vector<int> rev_path;
        for (size_t v = end_idx; v != start_idx; v = predecessors[v])
        {
            if (predecessors[v] == v) // цикл или ошибка
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
        result.algorithmName = useWeights ? "BGL A* (Multi-Param)" : "BGL A* (Uniform)";

        // многократный запуск для измерения времени (без восстановления пути)
        for (int run = 0; run < RUNS; ++run)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            std::vector<double> temp_distances(n, std::numeric_limits<double>::infinity());
            std::vector<BGLGraph::vertex_descriptor> temp_predecessors(n);
            
            // инициализация предшественников
            for (size_t i = 0; i < n; ++i)
                temp_predecessors[i] = i;

            try
            {
                auto temp_heuristic = AStarHeuristic<BGLGraph, double>(end_idx);
                auto temp_visitor = AStarGoalVisitor<BGLGraph::vertex_descriptor>(end_idx);

                boost::astar_search(
                    bgl_graph,
                    start_idx,
                    temp_heuristic,
                    boost::predecessor_map(&temp_predecessors[0])
                       .distance_map(&temp_distances[0])
                       .visitor(temp_visitor)
                );
            }
            catch (const found_goal&)
            {
                // путь найден - это нормально
            }
            catch (...)
            {
                // игнорируем ошибки при многократном запуске
            }
            
            auto endTime = std::chrono::high_resolution_clock::now();
            total_duration += (endTime - startTime);
        }

        // усредненное время
        result.executionTime = (total_duration.count() / RUNS) / 1000000.0;
        return result;
    }
}
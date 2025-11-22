#include "infrastructure/AStarPathFinder.h"
#include <limits>
#include <algorithm>
#include <iostream>

namespace Infrastructure
{
    Domain::PathResult AStarPathFinder::findShortestPath(
        const Domain::NetworkGraphPtr &graph,
        int start_id, int end_id)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        Domain::PathResult result;

        if (!graph || !graph->hasNode(start_id) || !graph->hasNode(end_id)) {
            result.success = false;
            result.errorMessage = "Start or end node not found";
            return result;
        }

        // структуры для A*
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
        std::unordered_map<int, double> g_score;
        std::unordered_map<int, int> came_from;

        // инициализация
        auto node_ids = graph->getAllNodeIds();
        for (int id : node_ids) {
            g_score[id] = std::numeric_limits<double>::infinity();
        }

        g_score[start_id] = 0.0;
        open_set.push({start_id, heuristic(start_id, end_id), 0.0});

        while (!open_set.empty()) {
            Node current = open_set.top();
            open_set.pop();

            int current_id = current.id;
            
            // если достигли цели
            if (current_id == end_id) {
                break;
            }

            // если нашли лучший путь к этой вершине после добавления в очередь
            if (current.g_score > g_score[current_id]) {
                continue;
            }

            // обработка соседей
            for (int neighbor_id : graph->getNeighbors(current_id)) {
                double edge_weight = 1.0; // версия без весов
                
                if (useWeights) {
                    // версия С весами
                    try {
                        edge_weight = graph->getEdgeWeight(current_id, neighbor_id, strategy);
                    } catch (...) {
                        edge_weight = 1.0; // fallback
                    }
                }

                double tentative_g_score = g_score[current_id] + edge_weight;

                if (tentative_g_score < g_score[neighbor_id]) {
                    // этот путь лучше, запоминаем его
                    came_from[neighbor_id] = current_id;
                    g_score[neighbor_id] = tentative_g_score;
                    double f_score = tentative_g_score + heuristic(neighbor_id, end_id);
                    open_set.push({neighbor_id, f_score, tentative_g_score});
                }
            }
        }

        // проверка, найден ли путь
        if (g_score[end_id] == std::numeric_limits<double>::infinity()) {
            result.success = false;
            result.errorMessage = "No path found (A*)";
            return result;
        }

        // восстановление пути
        std::vector<int> path;
        int current = end_id;
        while (current != start_id) {
            path.push_back(current);
            current = came_from[current];
        }
        path.push_back(start_id);
        std::reverse(path.begin(), path.end());

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        result.success = true;
        result.pathNodes = std::move(path);
        result.totalCost = g_score[end_id];
        result.executionTime = duration.count() / 1000.0; // convert to milliseconds
        result.algorithmName = useWeights ? 
            "A* (Multi-Param Weights)" : 
            "A* (Uniform Weights)";
            
        return result;
    }
}
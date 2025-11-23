#include "infrastructure/AStarPathFinder.h"
#include <limits>
#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <unordered_set>

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

        // ограничиваем максимальное количество посещенных узлов для больших графов
        const size_t MAX_VISITED_NODES = 10000;

        struct Node {
            int id;
            double f_score;
            double g_score;
            
            bool operator>(const Node& other) const {
                return f_score > other.f_score;
            }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
        std::unordered_map<int, double> g_score;
        std::unordered_map<int, int> came_from;
        std::unordered_set<int> closed_set;

        // инициализация
        g_score[start_id] = 0.0;
        open_set.push({start_id, heuristic(start_id, end_id), 0.0});

        size_t visited_count = 0;

        while (!open_set.empty() && visited_count < MAX_VISITED_NODES) {
            Node current = open_set.top();
            open_set.pop();

            int current_id = current.id;
            
            // если уже обработали этот узел
            if (closed_set.find(current_id) != closed_set.end()) {
                continue;
            }

            closed_set.insert(current_id);
            visited_count++;

            // если достигли цели
            if (current_id == end_id) {
                break;
            }

            // обработка соседей
            for (int neighbor_id : graph->getNeighbors(current_id)) {
                if (closed_set.find(neighbor_id) != closed_set.end()) {
                    continue;
                }

                double edge_weight = 1.0;
                
                if (useWeights) {
                    try {
                        edge_weight = graph->getEdgeWeight(current_id, neighbor_id, strategy);
                    } catch (...) {
                        edge_weight = 1.0;
                    }
                }

                double tentative_g_score = g_score[current_id] + edge_weight;

                // если нашли лучший путь к соседу
                if (g_score.find(neighbor_id) == g_score.end() || 
                    tentative_g_score < g_score[neighbor_id]) {
                    
                    came_from[neighbor_id] = current_id;
                    g_score[neighbor_id] = tentative_g_score;
                    double f_score = tentative_g_score + heuristic(neighbor_id, end_id);
                    open_set.push({neighbor_id, f_score, tentative_g_score});
                }
            }
        }

        // проверка, найден ли путь
        if (g_score.find(end_id) == g_score.end() || 
            g_score[end_id] == std::numeric_limits<double>::infinity()) {
            result.success = false;
            if (visited_count >= MAX_VISITED_NODES) {
                result.errorMessage = "Search limit exceeded (" + std::to_string(MAX_VISITED_NODES) + " nodes visited)";
            } else {
                result.errorMessage = "No path found between nodes " + 
                                    std::to_string(start_id) + " and " + std::to_string(end_id);
            }
            return result;
        }

        // восстановление пути
        std::vector<int> path;
        int current = end_id;
        while (current != start_id) {
            path.push_back(current);
            current = came_from[current];
            if (path.size() > 1000) { // защита от бесконечного цикла
                result.success = false;
                result.errorMessage = "Path too long or cycle detected";
                return result;
            }
        }
        path.push_back(start_id);
        std::reverse(path.begin(), path.end());

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        result.success = true;
        result.pathNodes = std::move(path);
        result.totalCost = g_score[end_id];
        result.executionTime = duration.count() / 1000.0;
        result.algorithmName = useWeights ? 
            "A* (Multi-Param Weights)" : 
            "A* (Uniform Weights)";
            
        return result;
    }
}
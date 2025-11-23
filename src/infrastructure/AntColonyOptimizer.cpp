#include "infrastructure/AntColonyOptimizer.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <queue>

namespace Infrastructure
{
    HeuristicResult AntColonyOptimizer::optimize(const Domain::NetworkGraphPtr &graph,
                                               const std::vector<std::pair<int, int>> &demands)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        HeuristicResult result;

        if (demands.empty() || !graph) {
            result.success = false;
            return result;
        }

        auto [start, end] = demands[0];

        if (!graph->hasNode(start) || !graph->hasNode(end)) {
            result.success = false;
            return result;
        }

        std::cout << "Запуск ACO: " << start << " → " << end 
                  << " (муравьев: " << antCount << ", итераций: " << iterations << ")\n";

        // инициализация феромонов и эвристик
        initializePheromones(graph);
        initializeHeuristics(graph);

        Ant global_best;
        global_best.path_cost = std::numeric_limits<double>::max();

        // основной цикл ACO
        for (size_t iter = 0; iter < iterations; ++iter) {
            std::vector<Ant> ants;
            
            // построение решений каждым муравьем
            for (size_t i = 0; i < antCount; ++i) {
                Ant ant = constructSolution(graph, start, end);
                if (!ant.path.empty() && ant.path.back() == end) {
                    ants.push_back(ant);
                    
                    // обновление глобально лучшего решения
                    if (ant.path_cost < global_best.path_cost) {
                        global_best = ant;
                    }
                }
            }

            // обновление феромонов
            if (!ants.empty()) {
                updatePheromones(ants);
            }

            // логирование прогресса
            if (iter % 20 == 0 && !global_best.path.empty()) {
                std::cout << "  Итерация " << iter << ": лучшая стоимость = " 
                          << global_best.path_cost << "\n";
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        if (!global_best.path.empty()) {
            result.success = true;
            result.objective = global_best.path_cost;
            result.executionTime = duration.count() / 1000.0;
            result.algorithmName = "Ant Colony Optimization";

            // преобразование пути
            for (size_t i = 0; i + 1 < global_best.path.size(); ++i) {
                result.path.emplace_back(global_best.path[i], global_best.path[i + 1]);
            }

            std::cout << "ACO завершен: cost=" << global_best.path_cost 
                      << ", time=" << result.executionTime << "ms"
                      << ", путь: " << global_best.path.size() << " узлов\n";
        } else {
            result.success = false;
        }

        return result;
    }

    void AntColonyOptimizer::initializePheromones(const Domain::NetworkGraphPtr &graph) {
        pheromone.clear();
        auto nodes = graph->getAllNodeIds();
        
        // инициализация феромонов небольшим значением
        double initial_pheromone = 1.0;
        for (int node : nodes) {
            auto neighbors = graph->getNeighbors(node);
            for (int neighbor : neighbors) {
                pheromone[node][neighbor] = initial_pheromone;
            }
        }
    }

    void AntColonyOptimizer::initializeHeuristics(const Domain::NetworkGraphPtr &graph) {
        heuristic.clear();
        auto nodes = graph->getAllNodeIds();
        
        // эвристическая информация = 1/стоимость_ребра
        for (int node : nodes) {
            auto neighbors = graph->getNeighbors(node);
            for (int neighbor : neighbors) {
                try {
                    double cost = graph->getEdgeWeight(node, neighbor, strategy);
                    heuristic[node][neighbor] = 1.0 / (cost + 0.1); // +0.1 чтобы избежать деления на 0
                } catch (...) {
                    heuristic[node][neighbor] = 0.1;
                }
            }
        }
    }

    AntColonyOptimizer::Ant AntColonyOptimizer::constructSolution(const Domain::NetworkGraphPtr &graph, 
                                                                 int start, int end) {
        Ant ant;
        ant.visited.insert(start);
        ant.path.push_back(start);
        ant.path_cost = 0.0;

        int current = start;
        
        const size_t MAX_PATH_LENGTH = 50;
        
        while (current != end && ant.path.size() < MAX_PATH_LENGTH) {
            int next = selectNextNode(ant, graph, end);
            if (next == -1) {
                break; // нет доступных соседей
            }
            
            // добавляем стоимость перехода
            try {
                double edge_cost = graph->getEdgeWeight(current, next, strategy);
                ant.path_cost += edge_cost;
            } catch (...) {
                break;
            }
            
            ant.path.push_back(next);
            ant.visited.insert(next);
            current = next;
        }
        
        return ant;
    }

    int AntColonyOptimizer::selectNextNode(const Ant& ant, const Domain::NetworkGraphPtr &graph, int end) {
        int current = ant.path.back();
        auto available = getAvailableNeighbors(ant, graph, current);
        
        if (available.empty()) {
            return -1;
        }
        
        // вычисление вероятностей перехода
        std::vector<double> probabilities;
        double sum = 0.0;
        
        for (int neighbor : available) {
            double prob = calculateTransitionProbability(ant, current, neighbor, graph);
            probabilities.push_back(prob);
            sum += prob;
        }
        
        if (sum == 0.0) {
            // случайный выбор если все вероятности нулевые
            std::uniform_int_distribution<size_t> dist(0, available.size() - 1);
            return available[dist(gen)];
        }
        
        // нормализация и рулеточный выбор
        std::uniform_real_distribution<double> dist(0.0, sum);
        double random_value = dist(gen);
        double cumulative = 0.0;
        
        for (size_t i = 0; i < available.size(); ++i) {
            cumulative += probabilities[i];
            if (random_value <= cumulative) {
                return available[i];
            }
        }
        
        return available.back();
    }

    double AntColonyOptimizer::calculateTransitionProbability(const Ant& ant, int current, int next, 
                                                             const Domain::NetworkGraphPtr &graph) {
        // формула ACO: (pheromone^alpha) * (heuristic^beta)
        double pheromone_value = pheromone[current][next];
        double heuristic_value = heuristic[current][next];
        
        return std::pow(pheromone_value, alpha) * std::pow(heuristic_value, beta);
    }

    void AntColonyOptimizer::updatePheromones(std::vector<Ant>& ants) {
        // испарение феромонов
        for (auto& [node, neighbors] : pheromone) {
            for (auto& [neighbor, value] : neighbors) {
                value *= (1.0 - evaporation);
            }
        }
        
        // обновление феромонов на основе качества решений
        for (auto& ant : ants) {
            double delta_pheromone = Q / ant.path_cost;
            
            for (size_t i = 0; i + 1 < ant.path.size(); ++i) {
                int from = ant.path[i];
                int to = ant.path[i + 1];
                
                if (pheromone.find(from) != pheromone.end() && 
                    pheromone[from].find(to) != pheromone[from].end()) {
                    pheromone[from][to] += delta_pheromone;
                }
            }
        }
    }

    std::vector<int> AntColonyOptimizer::getAvailableNeighbors(const Ant& ant, 
                                                              const Domain::NetworkGraphPtr &graph, 
                                                              int current) {
        std::vector<int> available;
        auto neighbors = graph->getNeighbors(current);
        
        for (int neighbor : neighbors) {
            if (ant.visited.find(neighbor) == ant.visited.end()) {
                available.push_back(neighbor);
            }
        }
        
        return available;
    }

    bool AntColonyOptimizer::isValidPath(const std::vector<int>& path, const Domain::NetworkGraphPtr &graph) {
        if (path.size() < 2) return false;
        
        for (size_t i = 0; i < path.size() - 1; ++i) {
            if (!graph->hasNode(path[i]) || !graph->hasNode(path[i + 1])) {
                return false;
            }
            auto neighbors = graph->getNeighbors(path[i]);
            if (std::find(neighbors.begin(), neighbors.end(), path[i + 1]) == neighbors.end()) {
                return false;
            }
        }
        return true;
    }

    double AntColonyOptimizer::calculatePathCost(const std::vector<int>& path, const Domain::NetworkGraphPtr &graph) {
        double total_cost = 0.0;
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            try {
                total_cost += graph->getEdgeWeight(path[i], path[i + 1], strategy);
            } catch (...) {
                return std::numeric_limits<double>::max();
            }
        }
        return total_cost;
    }
}
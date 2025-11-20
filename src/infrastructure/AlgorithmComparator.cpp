#include "infrastructure/AlgorithmComparator.h"
#include <iostream>
#include <iomanip>

namespace Infrastructure
{
    std::vector<AlgorithmComparison> AlgorithmComparator::compareAlgorithms(
        const Domain::NetworkGraphPtr &graph,
        const std::vector<std::pair<int, int>> &test_routes)
    {
        std::vector<AlgorithmComparison> results;

        for (const auto& route : test_routes) {
            auto [start, end] = route;
            
            std::cout << "Сравнение алгоритмов для маршрута " << start << " → " << end << ":\n";
            
            // тестируем разные варианты алгоритмов
            std::vector<std::unique_ptr<IPathFinder>> algorithms;
            algorithms.push_back(std::make_unique<BGLShortestPath>(false)); // дейкстра без весов
            algorithms.push_back(std::make_unique<BGLShortestPath>(true));  // дейкстра с весами
            algorithms.push_back(std::make_unique<AStarPathFinder>(false)); // A* без весов  
            algorithms.push_back(std::make_unique<AStarPathFinder>(true));  // A* с весами

            for (auto& algo : algorithms) {
                auto path_result = algo->findShortestPath(graph, start, end);
                
                AlgorithmComparison comp;
                comp.algorithmName = path_result.algorithmName;
                comp.executionTime = path_result.executionTime;
                comp.pathCost = path_result.totalCost;
                comp.pathLength = path_result.pathNodes.size();
                comp.success = path_result.success;
                
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            }
            std::cout << "\n";
        }

        return results;
    }

    void AlgorithmComparator::printComparisonTable(const std::vector<AlgorithmComparison> &results) {
        std::cout << "\n=== ИТОГОВОЕ СРАВНЕНИЕ АЛГОРИТМОВ ===\n";
        std::cout << std::setw(30) << "Algorithm" 
                  << std::setw(10) << "Success" 
                  << std::setw(12) << "Cost" 
                  << std::setw(8) << "Length" 
                  << std::setw(10) << "Time(ms)" << "\n";
        std::cout << std::string(70, '-') << "\n";
        
        for (const auto& result : results) {
            std::cout << std::setw(30) << result.algorithmName
                      << std::setw(10) << (result.success ? "OK" : "FAIL")
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.pathCost
                      << std::setw(8) << result.pathLength
                      << std::setw(10) << std::setprecision(1) << result.executionTime << "\n";
        }
    }
}
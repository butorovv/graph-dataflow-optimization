#include "infrastructure/AlgorithmComparator.h"
#include <iostream>
#include <iomanip>
#include <chrono>

namespace Infrastructure
{
    std::vector<AlgorithmComparison> AlgorithmComparator::compareAlgorithms(
        const Domain::NetworkGraphPtr &graph,
        const std::vector<std::pair<int, int>> &test_routes)
    {
        std::vector<AlgorithmComparison> results;

        std::cout << "ПОЛНОЕ СРАВНЕНИЕ АЛГОРИТМОВ (Точные + Эвристические)\n";
        
        for (const auto& route : test_routes) {
            auto [start, end] = route;
            if (start == end) continue;
            
            std::cout << "\nМАРШРУТ " << start << " → " << end << ":\n";
            std::cout << std::string(50, '-') << "\n";
            
            // ТОЧНЫЕ АЛГОРИТМЫ
            std::cout << "ТОЧНЫЕ АЛГОРИТМЫ:\n";
            
            // BGL Dijkstra - Uniform
            try {
                BGLShortestPath bglUniform(false);
                auto startTime = std::chrono::high_resolution_clock::now();
                auto result = bglUniform.findShortestPath(graph, start, end);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = result.algorithmName;
                comp.executionTime = duration.count() / 1000.0;
                comp.pathCost = result.totalCost;
                comp.pathLength = result.pathNodes.size();
                comp.success = result.success;
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            } catch (const std::exception& e) {
                std::cout << "  BGL Dijkstra (Uniform): FAIL Error: " << e.what() << "\n";
                
                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = "BGL Dijkstra (Uniform) - Error";
                comp.success = false;
                results.push_back(comp);
            }

            // BGL Dijkstra - Multi-Param
            try {
                BGLShortestPath bglMultiParam(true);
                auto startTime = std::chrono::high_resolution_clock::now();
                auto result = bglMultiParam.findShortestPath(graph, start, end);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = result.algorithmName;
                comp.executionTime = duration.count() / 1000.0;
                comp.pathCost = result.totalCost;
                comp.pathLength = result.pathNodes.size();
                comp.success = result.success;
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            } catch (const std::exception& e) {
                std::cout << "  BGL Dijkstra (Multi-Param): FAIL Error: " << e.what() << "\n";
                
                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = "BGL Dijkstra (Multi-Param) - Error";
                comp.success = false;
                results.push_back(comp);
            }

            // A* - Uniform
            try {
                AStarPathFinder astarUniform(false);
                auto startTime = std::chrono::high_resolution_clock::now();
                auto result = astarUniform.findShortestPath(graph, start, end);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = result.algorithmName;
                comp.executionTime = duration.count() / 1000.0;
                comp.pathCost = result.totalCost;
                comp.pathLength = result.pathNodes.size();
                comp.success = result.success;
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            } catch (const std::exception& e) {
                std::cout << "  A* (Uniform): FAIL Error: " << e.what() << "\n";
                
                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = "A* (Uniform) - Error";
                comp.success = false;
                results.push_back(comp);
            }

            // A* - Multi-Param
            try {
                AStarPathFinder astarMultiParam(true);
                auto startTime = std::chrono::high_resolution_clock::now();
                auto result = astarMultiParam.findShortestPath(graph, start, end);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = result.algorithmName;
                comp.executionTime = duration.count() / 1000.0;
                comp.pathCost = result.totalCost;
                comp.pathLength = result.pathNodes.size();
                comp.success = result.success;
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            } catch (const std::exception& e) {
                std::cout << "  A* (Multi-Param): FAIL Error: " << e.what() << "\n";
                
                AlgorithmComparison comp;
                comp.algorithmType = "Exact";
                comp.algorithmName = "A* (Multi-Param) - Error";
                comp.success = false;
                results.push_back(comp);
            }

            // ЭВРИСТИЧЕСКИЕ АЛГОРИТМЫ
            std::cout << "\nЭВРИСТИЧЕСКИЕ АЛГОРИТМЫ:\n";
            
            std::vector<std::pair<int, int>> single_demand = {{start, end}};
            
            // Генетический алгоритм
            try {
                GeneticAlgorithm geneticAlgo(50, 100, 0.15, 0.8, Domain::WeightCalculator::BALANCE_LOAD);
                auto startTime = std::chrono::high_resolution_clock::now();
                auto result = geneticAlgo.optimize(graph, single_demand);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

                AlgorithmComparison comp;
                comp.algorithmType = "Heuristic";
                comp.algorithmName = "Genetic Algorithm";
                comp.executionTime = duration.count() / 1000.0;
                comp.pathCost = result.objective;
                comp.pathLength = result.path.size();
                comp.success = result.success;
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            } catch (const std::exception& e) {
                std::cout << "  Genetic Algorithm: FAIL Error: " << e.what() << "\n";
                
                AlgorithmComparison comp;
                comp.algorithmType = "Heuristic";
                comp.algorithmName = "Genetic Algorithm - Error";
                comp.success = false;
                results.push_back(comp);
            }

            // Муравьиный алгоритм
            try {
                AntColonyOptimizer antAlgo(50, 100, 1.0, 2.0, 0.5, 100.0, Domain::WeightCalculator::BALANCE_LOAD);
                auto startTime = std::chrono::high_resolution_clock::now();
                auto result = antAlgo.optimize(graph, single_demand);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

                AlgorithmComparison comp;
                comp.algorithmType = "Heuristic";
                comp.algorithmName = "Ant Colony Optimization";
                comp.executionTime = duration.count() / 1000.0;
                comp.pathCost = result.objective;
                comp.pathLength = result.path.size();
                comp.success = result.success;
                results.push_back(comp);
                
                std::cout << "  " << comp.algorithmName << ": "
                          << (comp.success ? "OK" : "FAIL")
                          << " cost=" << comp.pathCost
                          << " length=" << comp.pathLength
                          << " time=" << comp.executionTime << "ms\n";
            } catch (const std::exception& e) {
                std::cout << "  Ant Colony Optimization: FAIL Error: " << e.what() << "\n";
                
                AlgorithmComparison comp;
                comp.algorithmType = "Heuristic";
                comp.algorithmName = "Ant Colony Optimization - Error";
                comp.success = false;
                results.push_back(comp);
            }
        }

        return results;
    }

    void AlgorithmComparator::printComparisonTable(const std::vector<AlgorithmComparison> &results) {
        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << "ПОЛНАЯ ТАБЛИЦА СРАВНЕНИЯ АЛГОРИТМОВ\n";
        std::cout << std::string(80, '=') << "\n";
        
        // Группируем по типам алгоритмов
        std::cout << "\nТОЧНЫЕ АЛГОРИТМЫ:\n";
        std::cout << std::setw(35) << "Algorithm" 
                  << std::setw(10) << "Success" 
                  << std::setw(12) << "Cost" 
                  << std::setw(8) << "Length" 
                  << std::setw(12) << "Time(ms)" << "\n";
        std::cout << std::string(77, '-') << "\n";
        
        for (const auto& result : results) {
            if (result.algorithmType == "Exact") {
                std::cout << std::setw(35) << result.algorithmName
                          << std::setw(10) << (result.success ? "OK" : "FAIL")
                          << std::setw(12) << std::fixed << std::setprecision(2) << result.pathCost
                          << std::setw(8) << result.pathLength
                          << std::setw(12) << std::setprecision(1) << result.executionTime << "\n";
            }
        }
        
        std::cout << "\nЭВРИСТИЧЕСКИЕ АЛГОРИТМЫ:\n";
        std::cout << std::setw(35) << "Algorithm" 
                  << std::setw(10) << "Success" 
                  << std::setw(12) << "Cost" 
                  << std::setw(8) << "Length" 
                  << std::setw(12) << "Time(ms)" << "\n";
        std::cout << std::string(77, '-') << "\n";
        
        for (const auto& result : results) {
            if (result.algorithmType == "Heuristic") {
                std::cout << std::setw(35) << result.algorithmName
                          << std::setw(10) << (result.success ? "OK" : "FAIL")
                          << std::setw(12) << std::fixed << std::setprecision(2) << result.pathCost
                          << std::setw(8) << result.pathLength
                          << std::setw(12) << std::setprecision(1) << result.executionTime << "\n";
            }
        }
        
        // Сводная статистика
        std::cout << "\nСВОДНАЯ СТАТИСТИКА:\n";
        int exactCount = 0, heuristicCount = 0;
        int exactSuccess = 0, heuristicSuccess = 0;
        
        for (const auto& result : results) {
            if (result.algorithmType == "Exact") {
                exactCount++;
                if (result.success) exactSuccess++;
            } else {
                heuristicCount++;
                if (result.success) heuristicSuccess++;
            }
        }
        
        std::cout << "Точные алгоритмы: " << exactSuccess << "/" << exactCount << " успешных (" 
                  << (exactSuccess * 100.0 / exactCount) << "%)\n";
        std::cout << "Эвристические алгоритмы: " << heuristicSuccess << "/" << heuristicCount << " успешных (" 
                  << (heuristicSuccess * 100.0 / heuristicCount) << "%)\n";
    }
}
#include <iostream>
#include <windows.h>
#include <memory>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <algorithm>

// core components
#include "domain/NetworkGraph.h"
#include "application/GraphAnalysisFactory.h"

// infrastructure
#include "infrastructure/AlgorithmComparator.h"
#include "infrastructure/BasicMonitor.h"
#include "infrastructure/SimpleStorage.h"
#include "infrastructure/BoostFlowSolver.h"

void setupConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void printBanner() {
    std::cout << "ПОЛНОЕ СРАВНЕНИЕ 6 АЛГОРИТМОВ\n";
    std::cout << "Граф: gr_1500.csv (300 узлов)\n";
    std::cout << "Алгоритмы: Дейкстра (2 версии), A* (2 версии), Генетический, Муравьиный\n\n";
}

int main() {
    setupConsole();
    printBanner();

    try {
        auto total_start_time = std::chrono::high_resolution_clock::now();

        // 1) загрузка графа
        std::cout << "ЗАГРУЗКА ГРАФА...\n";
        auto repo = Application::GraphAnalysisFactory::createRepository();
        auto graph = repo->loadGraph("data/datasets/gr_1500.csv");
        std::cout << "OK Успешно загружен: " << graph->getInfo() << "\n\n";

        // 2) базовый анализ
        std::cout << "БАЗОВЫЙ АНАЛИЗ СЕТИ:\n";
        Infrastructure::BasicMonitor::collectBasicMetrics(graph);
        std::cout << "\n";

        // 3) генерация тестовых маршрутов
        std::cout << "СОЗДАНИЕ ТЕСТОВЫХ МАРШРУТОВ...\n";
        auto nodes = graph->getAllNodeIds();
        std::vector<std::pair<int, int>> test_routes;
        
        // выбираем узлы с хорошей связностью
        std::vector<int> good_nodes;
        for (int i = 0; i < std::min(20, (int)nodes.size()); ++i) {
            if (graph->getNeighbors(nodes[i]).size() >= 3) {
                good_nodes.push_back(nodes[i]);
                if (good_nodes.size() >= 6) break;
            }
        }
        
        if (good_nodes.size() >= 4) {
            test_routes.push_back({good_nodes[0], good_nodes[1]});
            test_routes.push_back({good_nodes[2], good_nodes[3]});
            test_routes.push_back({good_nodes[0], good_nodes[4]});
            test_routes.push_back({good_nodes[1], good_nodes[5]});
        } else {
            // fallback
            for (int i = 0; i < std::min(4, (int)nodes.size() - 1); i += 2) {
                test_routes.push_back({nodes[i], nodes[i + 1]});
            }
        }
        
        std::cout << "OK Создано " << test_routes.size() << " тестовых маршрутов\n\n";

        // 4) полное сравнение 6 алгоритмов через AlgorithmComparator
        std::cout << "ЗАПУСК ПОЛНОГО СРАВНЕНИЯ 6 АЛГОРИТМОВ...\n";
        auto all_results = Infrastructure::AlgorithmComparator::compareAlgorithms(graph, test_routes);
        
        // 5) вывод полной таблицы сравнения
        Infrastructure::AlgorithmComparator::printComparisonTable(all_results);

        // 6) тестирование потоковых алгоритмов
        if (test_routes.size() >= 2) {
            std::cout << "\nТЕСТИРОВАНИЕ ПОТОКОВЫХ АЛГОРИТМОВ:\n";
            auto flow_solver = Application::GraphAnalysisFactory::createFlowSolver();
            
            for (size_t i = 0; i < std::min(test_routes.size(), size_t(2)); ++i) {
                const auto& route = test_routes[i];
                std::cout << "Маршрут " << route.first << " → " << route.second << ":\n";
                
                try {
                    auto result = flow_solver->solveMaxFlow(graph, route.first, route.second);
                    if (result.success) {
                        std::cout << "  Макс. поток: " << result.maxFlow 
                                  << ", время: " << result.totalCost << "ms\n";
                    } else {
                        std::cout << "  FAIL: " << result.errorMessage << "\n";
                    }
                } catch (const std::exception& e) {
                    std::cout << "  FAIL: " << e.what() << "\n";
                }
            }
        }

        // 7) тестирование отказоустойчивости
        if (nodes.size() >= 3) {
            std::cout << "\nТЕСТИРОВАНИЕ ОТКАЗОУСТОЙЧИВОСТИ:\n";
            
            std::vector<int> test_nodes = {nodes[1]};
            for (int node_id : test_nodes) {
                if (graph->hasNode(node_id)) {
                    Infrastructure::BasicMonitor::simulateNodeFailure(graph, node_id);
                }
            }
            
            // проверяем связность после отказов
            if (!test_routes.empty()) {
                std::vector<std::pair<int, int>> connectivity_test;
                for (size_t i = 0; i < std::min(test_routes.size(), size_t(2)); ++i) {
                    connectivity_test.push_back(test_routes[i]);
                }
                Infrastructure::BasicMonitor::analyzeConnectivity(graph, connectivity_test);
            }
            std::cout << "\n";
        }

        // 8) сохранение результатов
        std::cout << "СОХРАНЕНИЕ РЕЗУЛЬТАТОВ...\n";
        Infrastructure::SimpleStorage::saveExperimentResults("full_algorithm_comparison.csv", all_results);
        Infrastructure::SimpleStorage::saveGraphInfo(graph, "full_test_results.txt");
        Infrastructure::SimpleStorage::logEvent("Полное сравнение 6 алгоритмов завершено");
        std::cout << "OK Результаты сохранены в файлы:\n";
        std::cout << "- full_algorithm_comparison.csv\n";
        std::cout << "- full_test_results.txt\n";
        std::cout << "- experiment_log.txt\n\n";

        auto total_end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(total_end_time - total_start_time);

        std::cout << "ПОЛНОЕ СРАВНЕНИЕ 6 АЛГОРИТМОВ ЗАВЕРШЕНО!\n";
        std::cout << "Общее время выполнения: " << total_duration.count() << " секунд\n";
        std::cout << "Протестированы алгоритмы:\n";
        std::cout << "- Дейкстра (Uniform)\n";
        std::cout << "- Дейкстра (Multi-Param)\n"; 
        std::cout << "- A* (Uniform)\n";
        std::cout << "- A* (Multi-Param)\n";
        std::cout << "- Genetic Algorithm\n";
        std::cout << "- Ant Colony Optimization\n";
        std::cout << "Количество маршрутов: " << test_routes.size() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "FAIL Критическая ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
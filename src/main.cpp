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

// config
#include "config/StrategyConfig.h"

void setupConsole()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

// генерация дорожного графа в памяти
Domain::NetworkGraphPtr generateAstarFriendlyGraph(int numNodes = 5000)
{
    auto graph = std::make_shared<Domain::NetworkGraph>();

    // создаем узлы
    for (int i = 0; i < numNodes; ++i)
    {
        graph->addNode(i);
    }

    // создаем «дорожные» связи, линейная цепочка + короткие ответвления
    for (int i = 0; i < numNodes; ++i)
    {
        if (i + 1 < numNodes)
            graph->addEdge(i, i + 1, 1.0);
        if (i + 2 < numNodes)
            graph->addEdge(i, i + 2, 1.2);
        if (i + 5 < numNodes)
            graph->addEdge(i, i + 5, 1.5);
    }

    // добавляем длинные кросс-связи для агрессивных маршрутов
    for (int i = 0; i < numNodes; i += 100)
    {
        if (i + 300 < numNodes)
            graph->addEdge(i, i + 300, 2.0);
        if (i + 500 < numNodes)
            graph->addEdge(i, i + 500, 2.5);
    }

    return graph;
}

// CLI выбор графа
std::string selectGraphFromCLI()
{
    std::cout << "🎯 ВЫБОР ГРАФА ДЛЯ ТЕСТИРОВАНИЯ\n";
    std::cout << "================================\n";
    std::cout << "1. gr_1500.csv (300 узлов) - Социальная сеть\n";
    std::cout << "2. astar_friendly.txt (200 узлов) - Дорожная сеть\n";
    std::cout << "3. memory:astar_friendly - Генерация графа A* в памяти\n";
    std::cout << "4. custom - Указать свой файл\n";
    std::cout << "\nВыберите граф (1-5): ";

    int choice;
    std::cin >> choice;

    switch (choice)
    {
    case 1:
        return "data/datasets/gr_1500.csv";
    case 2:
        return "data/datasets/astar_friendly.csv";
    case 3:
        return "memory:astar_friendly";
    case 4:
    {
        std::cout << "Введите путь к файлу графа: ";
        std::string custom_path;
        std::cin >> custom_path;
        return custom_path;
    }
    default:
        std::cout << "Неверный выбор, используем gr_1500.csv по умолчанию\n";
        return "data/datasets/gr_1500.csv";
    }
}

// получение тестовых маршрутов
std::vector<std::pair<int, int>> getTestRoutesForGraph(const std::string &graph_file,
                                                       const Domain::NetworkGraphPtr &graph)
{
    if (graph_file.find("astar_friendly") != std::string::npos)
    {
        // агрессивные тестовые маршруты
        return {
            {0, 2300},
            {10, 2000},
            {50, 1800},
            {300, 1200},
            {120, 300}};
    }
    else
    {
        auto nodes = graph->getAllNodeIds();
        std::vector<std::pair<int, int>> routes;
        std::vector<int> good_nodes;

        for (int i = 0; i < std::min(20, (int)nodes.size()); ++i)
        {
            if (graph->getNeighbors(nodes[i]).size() >= 3)
            {
                good_nodes.push_back(nodes[i]);
                if (good_nodes.size() >= 6)
                    break;
            }
        }

        if (good_nodes.size() >= 4)
        {
            routes.push_back({good_nodes[0], good_nodes[1]});
            routes.push_back({good_nodes[2], good_nodes[3]});
            routes.push_back({good_nodes[0], good_nodes[4]});
            routes.push_back({good_nodes[1], good_nodes[5]});
        }
        else
        {
            for (int i = 0; i < std::min(4, (int)nodes.size() - 1); i += 2)
            {
                routes.push_back({nodes[i], nodes[i + 1]});
            }
        }
        return routes;
    }
}

// описание графа
std::string getGraphDescription(const std::string &graph_file)
{
    if (graph_file.find("astar_friendly") != std::string::npos)
        return "astar_friendly.txt (200 узлов) - ДОРОЖНАЯ СЕТЬ";
    else if (graph_file.find("gr_1500") != std::string::npos)
        return "gr_1500.csv (300 узлов) - Социальная сеть";
    else if (graph_file.find("small_test") != std::string::npos)
        return "small_test.csv (50 узлов) - Тестовый граф";
    else
        return graph_file + " - Пользовательский граф";
}

// CLI выбор стратегии
Config::StrategySettings selectStrategyFromCLI()
{
    std::cout << "🎯 ВЫБОР СТРАТЕГИИ ВЕСОВ\n";
    std::cout << "=========================\n";
    std::cout << "1. Latency Optimized    - минимизация задержки (VoIP, игры)\n";
    std::cout << "2. Bandwidth Optimized  - максимизация пропускной способности (стриминг)\n";
    std::cout << "3. Balanced             - балансировка нагрузки (универсальный)\n";
    std::cout << "4. Cost Optimized       - минимизация стоимости (бюджетный)\n";
    std::cout << "5. Adaptive             - адаптивная стратегия (интеллектуальный)\n";
    std::cout << "6. Mixed                - смешанный подход (разные стратегии)\n";
    std::cout << "\nВыберите стратегию (1-6): ";

    int choice;
    std::cin >> choice;

    switch (choice)
    {
    case 1:
        return Config::StrategySettings::createLatencyOptimized();
    case 2:
        return Config::StrategySettings::createBandwidthOptimized();
    case 3:
        return Config::StrategySettings::createBalanced();
    case 4:
        return Config::StrategySettings::createCostOptimized();
    case 5:
        return Config::StrategySettings::createAdaptive();
    case 6:
        return Config::StrategySettings::createMixed();
    default:
        std::cout << "Неверный выбор, используем Latency Optimized по умолчанию\n";
        return Config::StrategySettings::createLatencyOptimized();
    }
}

int main()
{
    setupConsole();

    try
    {
        std::string graph_file = selectGraphFromCLI();
        std::string graph_description = getGraphDescription(graph_file);

        std::cout << "\n"
                  << std::string(50, '=') << "\n";
        std::cout << "ПОЛНОЕ СРАВНЕНИЕ 6 АЛГОРИТМОВ\n";
        std::cout << "Граф: " << graph_description << "\n\n";

        auto strategies = selectStrategyFromCLI();
        std::cout << "✅ Выбрана конфигурация: " << strategies.getDescription() << "\n\n";

        auto total_start_time = std::chrono::high_resolution_clock::now();

        Domain::NetworkGraphPtr graph;

        if (graph_file == "memory:astar_friendly")
        {
            std::cout << "ГЕНЕРАЦИЯ ГРАФА В ПАМЯТИ...\n";
            graph = generateAstarFriendlyGraph();
            std::cout << "✅ Сгенерирован граф: " << graph->getAllNodeIds().size() << " узлов\n\n";
        }
        else
        {
            auto repo = Application::GraphAnalysisFactory::createRepository();
            graph = repo->loadGraph(graph_file);
            std::cout << "✅ Успешно загружен: " << graph->getInfo() << "\n\n";
        }

        // базовый анализ
        std::cout << "БАЗОВЫЙ АНАЛИЗ СЕТИ:\n";
        Infrastructure::BasicMonitor::collectBasicMetrics(graph);
        std::cout << "\n";

        // генерация тестовых маршрутов
        std::cout << "СОЗДАНИЕ ТЕСТОВЫХ МАРШРУТОВ...\n";
        auto test_routes = getTestRoutesForGraph(graph_file, graph);
        std::cout << "✅ Создано " << test_routes.size() << " тестовых маршрутов\n";
        std::cout << "Маршруты: ";
        for (size_t i = 0; i < test_routes.size(); ++i)
        {
            std::cout << test_routes[i].first << "→" << test_routes[i].second;
            if (i < test_routes.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n\n";

        // сравнение алгоритмов
        std::cout << "ЗАПУСК ПОЛНОГО СРАВНЕНИЯ 6 АЛГОРИТМОВ...\n";
        auto all_results = Infrastructure::AlgorithmComparator::compareAlgorithms(graph, test_routes, strategies);
        Infrastructure::AlgorithmComparator::printComparisonTable(all_results);

        // сохранение результатов
        Infrastructure::SimpleStorage::saveExperimentResults("full_algorithm_comparison.csv", all_results);
        Infrastructure::SimpleStorage::saveGraphInfo(graph, "full_test_results.txt");
        Infrastructure::SimpleStorage::logEvent("Полное сравнение 6 алгоритмов завершено. Граф: " + graph_description + ", Стратегия: " + strategies.getDescription());
        std::cout << "\n✅ Результаты сохранены.\n";

        auto total_end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(total_end_time - total_start_time);
        std::cout << "Общее время выполнения: " << total_duration.count() << " секунд\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "❌ Критическая ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

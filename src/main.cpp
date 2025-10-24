#include <iostream>
#include <windows.h>
#include <memory>
#include <chrono>
#include <vector>
#include <limits>
#include "infrastructure/FileGraphRepository.h"
#include "application/GraphAnalysisService.h"
#include "infrastructure/BoostFlowSolver.h"

void setupRussianConsole()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

int main()
{
    setupRussianConsole();
    try
    {
        std::cout << "=== Анализ Facebook Social Network ===\n";
        std::cout << "🎯 Загрузка реального датасета: 4,039 пользователей, 88,234 связей\n\n";

        // загрузка Facebook датасета
        auto load_start = std::chrono::high_resolution_clock::now();

        Infrastructure::FileGraphRepository repo;
        auto graph = repo.loadGraph("data/datasets/ego-Facebook.csv");

        auto load_end = std::chrono::high_resolution_clock::now();
        auto load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(load_end - load_start);

        std::cout << "⏰ Время загрузки: " << load_duration.count() << " мс\n";
        std::cout << "📈 " << graph->getInfo() << "\n\n";

        // базовая статистика
        int node_count = graph->getNodeCount();
        int edge_count = 0;
        for (int node_id : graph->getAllNodeIds())
        {
            edge_count += graph->getNeighbors(node_id).size();
        }

        std::cout << "--- СТАТИСТИКА СЕТИ ---\n";
        std::cout << "• Пользователей: " << node_count << "\n";
        std::cout << "• Дружеских связей: " << edge_count << "\n";
        std::cout << "• Средняя степень: " << (2.0 * edge_count) / node_count << "\n\n";

        // параллельный поиск кратчайших путей
        std::cout << "--- ПОИСК ПУТЕЙ МЕЖДУ ПОЛЬЗОВАТЕЛЯМИ ---\n";
        std::vector<std::pair<int, int>> routes = {
            {0, 100},
            {0, 500},
            {0, 1000},
            {100, 2000},
            {500, 3000}};

        auto parallel_start = std::chrono::high_resolution_clock::now();
        Application::GraphAnalysisService service;
        auto results = service.analyzePathsParallel(graph, routes);
        auto parallel_end = std::chrono::high_resolution_clock::now();
        auto parallel_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parallel_end - parallel_start);

        int success_count = 0;
        for (size_t i = 0; i < results.size(); ++i)
        {
            std::cout << "👤 Маршрут " << routes[i].first << " → " << routes[i].second << ": ";
            if (results[i].success)
            {
                success_count++;
                std::cout << "✅ найдено! Длина = " << results[i].totalCost
                          << ", шагов = " << results[i].pathNodes.size() << "\n";
            }
            else
            {
                std::cout << "❌ " << results[i].errorMessage << "\n";
            }
        }

        std::cout << "\n--- РЕЗЮМЕ ПОИСКА ПУТЕЙ ---\n";
        std::cout << "• Найдено путей: " << success_count << "/" << routes.size() << "\n";
        std::cout << "• Время обработки: " << parallel_duration.count() << " мс\n";
        std::cout << "• Успешность: " << (success_count * 100.0 / routes.size()) << "%\n\n";

        // анализ максимального потока и минимальной стоимости
        std::cout << "--- АНАЛИЗ ПОТОКОВЫХ АЛГОРИТМОВ ---\n";

        Infrastructure::BoostFlowSolver flowSolver;

        // выберем одну пару для анализа потока (например, 0 - 1000)
        int source = 0, sink = 1000;

        if (!graph->hasNode(source) || !graph->hasNode(sink))
        {
            std::cout << "⚠️  Узлы " << source << " или " << sink << " отсутствуют — пропуск анализа потока.\n\n";
        }
        else
        {
            // максимальный поток
            std::cout << "📊 Максимальный поток (" << source << " → " << sink << "):\n";
            auto maxFlowResult = flowSolver.solveMaxFlow(graph, source, sink);
            if (maxFlowResult.success)
            {
                std::cout << "  ✅ Макс. поток: " << maxFlowResult.maxFlow << "\n";
                std::cout << "  ⏱️ Время: " << maxFlowResult.totalCost << " мс\n";
                std::cout << "  🧠 Алгоритм: " << maxFlowResult.algorithmName << "\n";
            }
            else
            {
                std::cout << "  ❌ Ошибка: " << maxFlowResult.errorMessage << "\n";
            }

            // минимальная стоимость максимального потока
            std::cout << "\n📊 Мин. стоимость макс. потока (" << source << " → " << sink << "):\n";
            auto minCostResult = flowSolver.solveMinCostMaxFlow(graph, source, sink);
            if (minCostResult.success)
            {
                std::cout << "  ✅ Макс. поток: " << minCostResult.maxFlow << "\n";
                std::cout << "  ⏱️ Время: " << minCostResult.totalCost << " мс\n";
                std::cout << "  🧠 Алгоритм: " << minCostResult.algorithmName << "\n";
            }
            else
            {
                std::cout << "  ❌ Ошибка: " << minCostResult.errorMessage << "\n";
            }
        }

        std::cout << "\n🎉 Анализ Facebook сети завершен!\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "❌ Ошибка: " << e.what() << "\n";
        std::cerr << "💡 Решение: Скачай датасет с https://snap.stanford.edu/data/facebook_combined.txt.gz\n";
        std::cerr << "           Распакуй и переименуй в 'ego-Facebook.csv' в папке data/datasets/\n";
        return 1;
    }

    std::cout << "\nPress Enter to exit...\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
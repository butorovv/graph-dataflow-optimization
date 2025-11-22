#include "infrastructure/GeneticAlgorithm.h"
#include <chrono>
#include <iostream>

namespace Infrastructure
{
    HeuristicResult GeneticAlgorithm::optimize(const Domain::NetworkGraphPtr &graph,
                                             const std::vector<std::pair<int, int>> &demands)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        HeuristicResult result;

        if (demands.empty()) {
            result.success = false;
            result.errorMessage = "No demands provided";
            return result;
        }

        // Фокусируемся на первом требовании для демонстрации
        auto [start, end] = demands[0];

        // Инициализация популяции
        auto population = initializePopulation(graph, start, end);
        if (population.empty()) {
            result.success = false;
            result.errorMessage = "Cannot initialize population";
            return result;
        }

        // Основной цикл генетического алгоритма
        for (size_t gen = 0; gen < generations; ++gen) {
            // Оценка приспособленности
            for (auto &chromosome : population) {
                chromosome.fitness = calculateFitness(chromosome, graph);
            }

            // Сортировка по приспособленности
            std::sort(population.begin(), population.end());

            // Селекция и скрещивание (упрощенная версия)
            std::vector<Chromosome> new_population;
            
            // Элитизм: сохраняем лучшие особи
            size_t elite_count = populationSize / 10;
            for (size_t i = 0; i < elite_count; ++i) {
                new_population.push_back(population[i]);
            }

            // Заполняем остальную популяцию потомками
            std::uniform_int_distribution<> dist(0, elite_count - 1);
            while (new_population.size() < populationSize) {
                auto &parent1 = population[dist(gen)];
                auto &parent2 = population[dist(gen)];
                auto child = crossover(parent1, parent2);
                mutate(child, graph);
                new_population.push_back(child);
            }

            population = std::move(new_population);
        }

        // Выбор лучшего решения
        auto best = *std::min_element(population.begin(), population.end());
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        result.success = true;
        result.objective = best.fitness;
        // Преобразование пути в формат результата
        for (size_t i = 0; i + 1 < best.path.size(); ++i) {
            result.path.emplace_back(best.path[i], best.path[i + 1]);
        }
        result.executionTime = duration.count() / 1000.0;
        result.algorithmName = "Genetic Algorithm";

        return result;
    }

    std::vector<GeneticAlgorithm::Chromosome> GeneticAlgorithm::initializePopulation(
        const Domain::NetworkGraphPtr &graph, int start, int end) 
    {
        std::vector<Chromosome> population;
        population.reserve(populationSize);

        for (size_t i = 0; i < populationSize; ++i) {
            auto path = generateRandomPath(graph, start, end);
            if (!path.empty()) {
                population.push_back({path, 0.0});
            }
        }

        return population;
    }

    double GeneticAlgorithm::calculateFitness(const Chromosome &chromosome, 
                                            const Domain::NetworkGraphPtr &graph) 
    {
        double total_cost = 0.0;
        for (size_t i = 0; i + 1 < chromosome.path.size(); ++i) {
            try {
                double weight = graph->getEdgeWeight(chromosome.path[i], chromosome.path[i + 1], strategy);
                total_cost += weight;
            } catch (...) {
                return std::numeric_limits<double>::max(); // Штраф за невалидный путь
            }
        }
        return total_cost;
    }

    // ... остальные методы генетического алгоритма ...
}
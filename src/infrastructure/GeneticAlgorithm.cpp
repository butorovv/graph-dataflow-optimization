#include "infrastructure/GeneticAlgorithm.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <algorithm>
#include <random>
#include <queue>
#include <unordered_map>

namespace Infrastructure
{
    HeuristicResult GeneticAlgorithm::optimize(const Domain::NetworkGraphPtr &graph,
                                               const std::vector<std::pair<int, int>> &demands)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        HeuristicResult result;

        // проверка входных данных
        if (demands.empty() || !graph)
        {
            std::cout << "FAIL Ошибка: пустые входные данные\n";
            result.success = false;
            return result;
        }

        auto [start, end] = demands[0];

        if (!graph->hasNode(start) || !graph->hasNode(end))
        {
            std::cout << "FAIL Ошибка: узлы " << start << " или " << end << " не найдены\n";
            result.success = false;
            return result;
        }

        std::cout << "Запуск оптимизированного ГА: " << start << " → " << end << "\n";

        // инициализация популяции
        auto population = initializePopulation(graph, start, end);
        if (population.empty())
        {
            std::cout << "FAIL Ошибка: не удалось инициализировать популяцию\n";
            result.success = false;
            return result;
        }

        std::cout << "OK Популяция инициализирована: " << population.size() << " особей\n";

        Chromosome global_best;
        global_best.fitness = std::numeric_limits<double>::max();

        // основной цикл генетического алгоритма
        for (size_t gen_count = 0; gen_count < generations; ++gen_count)
        {
            // оценка приспособленности
            int valid_count = 0;
            for (auto &chromosome : population)
            {
                if (!chromosome.valid)
                {
                    chromosome.fitness = calculateFitness(chromosome, graph, start, end);
                    chromosome.valid = true;
                    if (chromosome.fitness < std::numeric_limits<double>::max())
                    {
                        valid_count++;

                        // обновление глобально лучшего решения
                        if (chromosome.fitness < global_best.fitness)
                        {
                            global_best = chromosome;
                        }
                    }
                }
            }

            if (valid_count == 0)
            {
                std::cout << "FAIL Все особи невалидны, завершение\n";
                break;
            }

            // сортировка по приспособленности
            std::sort(population.begin(), population.end());

            // логирование прогресса
            if (gen_count % 10 == 0)
            {
                std::cout << "Поколение " << gen_count << ": лучший fitness = "
                          << population[0].fitness << ", valid = " << valid_count << "/" << population.size() << "\n";
            }

            // ранний выход при хорошем решении
            if (gen_count > 20 && population[0].fitness < 50.0)
            {
                std::cout << "Ранний выход на поколении " << gen_count
                          << "с fitness=" << population[0].fitness << "\n";
                break;
            }

            // создание новой популяции
            std::vector<Chromosome> new_population;

            // элитизм - сохраняем лучшие 20%
            size_t elite_count = std::max(size_t(2), populationSize / 5);
            for (size_t i = 0; i < elite_count && i < population.size(); ++i)
            {
                if (population[i].fitness < std::numeric_limits<double>::max())
                {
                    new_population.push_back(population[i]);
                }
            }

            // турнирная селекция и кроссовер
            std::uniform_int_distribution<size_t> tournament_dist(0, population.size() - 1);
            std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

            while (new_population.size() < populationSize)
            {
                // турнирная селекция 2x2
                size_t idx1 = tournament_dist(gen);
                size_t idx2 = tournament_dist(gen);
                auto &parent1 = (population[idx1].fitness < population[idx2].fitness) ? population[idx1] : population[idx2];

                idx1 = tournament_dist(gen);
                idx2 = tournament_dist(gen);
                auto &parent2 = (population[idx1].fitness < population[idx2].fitness) ? population[idx1] : population[idx2];

                Chromosome child;

                // скрещивание с вероятностью crossoverRate
                if (prob_dist(gen) < crossoverRate &&
                    parent1.path.size() >= 2 && parent2.path.size() >= 2 &&
                    isValidPath(parent1.path, graph, start, end) &&
                    isValidPath(parent2.path, graph, start, end))
                {
                    child = crossover(parent1, parent2, graph, start, end);
                }
                else
                {
                    // клонирование лучшего родителя
                    child = (parent1.fitness < parent2.fitness) ? parent1 : parent2;
                }
                child.valid = false;

                // мутация с вероятностью mutationRate
                if (prob_dist(gen) < mutationRate && child.path.size() >= 2)
                {
                    mutate(child, graph, start, end);
                    child.valid = false;
                }

                new_population.push_back(child);
            }

            population = std::move(new_population);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

        // используем глобально лучшее решение
        if (global_best.fitness < std::numeric_limits<double>::max() &&
            !global_best.path.empty() && global_best.path.back() == end)
        {

            result.success = true;
            result.objective = global_best.fitness;
            result.executionTime = duration.count() / 1000000.0;
            result.algorithmName = "Optimized Genetic Algorithm";

            // преобразование пути
            for (size_t i = 0; i + 1 < global_best.path.size(); ++i)
            {
                result.path.emplace_back(global_best.path[i], global_best.path[i + 1]);
            }

            std::cout << "OK ГА завершен: cost=" << global_best.fitness
                      << ", time=" << result.executionTime << "ms"
                      << ", путь: " << global_best.path.size() << " узлов\n";
        }
        else
        {
            std::cout << "FAIL ГА не нашел допустимого решения\n";
            result.success = false;
        }

        return result;
    }

    std::vector<GeneticAlgorithm::Chromosome> GeneticAlgorithm::initializePopulation(
        const Domain::NetworkGraphPtr &graph, int start, int end)
    {
        std::vector<Chromosome> population;
        population.reserve(populationSize);

        int success_count = 0;
        const int MAX_ATTEMPTS = populationSize * 3;

        for (int attempt = 0; attempt < MAX_ATTEMPTS && population.size() < populationSize; ++attempt)
        {
            std::vector<int> path;

            // чередуем стратегии для разнообразия
            if (attempt % 3 == 0)
            {
                path = generateRandomPathBFS(graph, start, end);
            }
            else if (attempt % 3 == 1)
            {
                path = generateGreedyPath(graph, start, end);
            }
            else
            {
                path = generateRandomPathDFS(graph, start, end);
            }

            if (!path.empty() && isValidPath(path, graph, start, end) && path.back() == end)
            {
                population.push_back({path, 0.0, false});
                success_count++;
            }
        }

        std::cout << "Сгенерировано " << success_count << "/" << populationSize << " валидных особей\n";
        return population;
    }

    double GeneticAlgorithm::calculateFitness(const Chromosome &chromosome,
                                              const Domain::NetworkGraphPtr &graph,
                                              int start, int end)
    {
        // проверка валидности пути
        if (chromosome.path.empty() ||
            chromosome.path[0] != start ||
            chromosome.path.back() != end ||
            !isValidPath(chromosome.path, graph, start, end))
        {
            return std::numeric_limits<double>::max();
        }

        return calculatePathCost(chromosome.path, graph);
    }

    GeneticAlgorithm::Chromosome GeneticAlgorithm::crossover(const Chromosome &parent1,
                                                             const Chromosome &parent2,
                                                             const Domain::NetworkGraphPtr &graph,
                                                             int start, int end)
    {
        // упрощенный одноточечный кроссовер
        if (parent1.path.size() < 3 || parent2.path.size() < 3)
        {
            return (parent1.fitness < parent2.fitness) ? parent1 : parent2;
        }

        std::uniform_int_distribution<size_t> dist(1, std::min(parent1.path.size(), parent2.path.size()) - 2);
        size_t crossover_point = dist(gen);

        // берем первую часть от parent1
        std::vector<int> child_path;
        child_path.insert(child_path.end(), parent1.path.begin(), parent1.path.begin() + crossover_point);

        // пытаемся найти продолжение из parent2
        int last_node = child_path.back();
        auto it = std::find(parent2.path.begin(), parent2.path.end(), last_node);

        if (it != parent2.path.end() && it + 1 != parent2.path.end())
        {
            // добавляем оставшуюся часть из parent2
            child_path.insert(child_path.end(), it + 1, parent2.path.end());
        }
        else
        {
            // если не нашли соединение, используем жадный алгоритм
            auto continuation = generateGreedyPath(graph, last_node, end);
            if (!continuation.empty())
            {
                child_path.insert(child_path.end(), continuation.begin() + 1, continuation.end());
            }
            else
            {
                // fallback - возвращаем лучшего родителя
                return (parent1.fitness < parent2.fitness) ? parent1 : parent2;
            }
        }

        return {child_path, 0.0, false};
    }

    void GeneticAlgorithm::mutate(Chromosome &chromosome,
                                  const Domain::NetworkGraphPtr &graph,
                                  int start, int end)
    {
        if (chromosome.path.size() < 3)
            return;

        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> pos_dist(1, chromosome.path.size() - 2);

        // разные типы мутаций
        double mutation_type = prob_dist(gen);

        if (mutation_type < 0.4)
        {
            // мутация заменой участка пути
            size_t pos = pos_dist(gen);
            if (pos < chromosome.path.size() - 1)
            {
                int from = chromosome.path[pos];
                int to = chromosome.path[pos + 1];

                // пытаемся найти альтернативный путь
                auto new_segment = generateRandomPathBFS(graph, from, to);
                if (!new_segment.empty() && new_segment.size() > 1)
                {
                    // заменяем сегмент
                    chromosome.path.erase(chromosome.path.begin() + pos, chromosome.path.begin() + pos + 2);
                    chromosome.path.insert(chromosome.path.begin() + pos,
                                           new_segment.begin(), new_segment.end());
                }
            }
        }
        else if (mutation_type < 0.7)
        {
            // мутация удалением узла (если есть прямое соединение)
            size_t pos = pos_dist(gen);
            if (pos > 0 && pos < chromosome.path.size() - 1)
            {
                int prev = chromosome.path[pos - 1];
                int next = chromosome.path[pos + 1];

                // проверяем, есть ли прямое соединение
                auto neighbors = graph->getNeighbors(prev);
                if (std::find(neighbors.begin(), neighbors.end(), next) != neighbors.end())
                {
                    chromosome.path.erase(chromosome.path.begin() + pos);
                }
            }
        }
        else
        {
            // мутация перестановкой двух соседних узлов
            size_t pos = pos_dist(gen);
            if (pos < chromosome.path.size() - 1)
            {
                std::swap(chromosome.path[pos], chromosome.path[pos + 1]);
            }
        }
    }

    // быстрая генерация пути с помощью BFS
    std::vector<int> GeneticAlgorithm::generateRandomPathBFS(const Domain::NetworkGraphPtr &graph,
                                                             int start, int end)
    {
        if (start == end)
            return {start};

        std::queue<std::vector<int>> paths;
        std::unordered_set<int> visited;
        paths.push({start});
        visited.insert(start);

        const size_t MAX_PATHS = 5000;
        size_t paths_explored = 0;

        while (!paths.empty() && paths_explored < MAX_PATHS)
        {
            auto current_path = paths.front();
            paths.pop();
            paths_explored++;

            int current_node = current_path.back();

            if (current_node == end)
            {
                return current_path;
            }

            auto neighbors = graph->getNeighbors(current_node);

            // ограничиваем количество проверяемых соседей для производительности
            size_t neighbors_to_check = std::min(size_t(5), neighbors.size());
            for (size_t i = 0; i < neighbors_to_check; ++i)
            {
                int neighbor = neighbors[i];
                if (visited.find(neighbor) == visited.end())
                {
                    visited.insert(neighbor);
                    auto new_path = current_path;
                    new_path.push_back(neighbor);
                    paths.push(new_path);

                    // ограничение длины пути
                    if (new_path.size() > 20)
                        break;
                }
            }
        }

        return {};
    }

    // генерация пути с помощью DFS (для разнообразия)
    std::vector<int> GeneticAlgorithm::generateRandomPathDFS(const Domain::NetworkGraphPtr &graph,
                                                             int start, int end)
    {
        if (start == end)
            return {start};

        std::vector<int> path;
        std::unordered_set<int> visited;

        std::function<bool(int)> dfs = [&](int current) -> bool
        {
            if (current == end)
            {
                path.push_back(current);
                return true;
            }

            if (visited.find(current) != visited.end() || path.size() > 15)
            {
                return false;
            }

            visited.insert(current);
            path.push_back(current);

            auto neighbors = graph->getNeighbors(current);
            std::vector<int> shuffled = neighbors;
            std::shuffle(shuffled.begin(), shuffled.end(), gen);

            for (int neighbor : shuffled)
            {
                if (dfs(neighbor))
                {
                    return true;
                }
            }

            path.pop_back();
            return false;
        };

        if (dfs(start))
        {
            return path;
        }
        return {};
    }

    // жадная генерация пути
    std::vector<int> GeneticAlgorithm::generateGreedyPath(const Domain::NetworkGraphPtr &graph,
                                                          int start, int end)
    {
        std::vector<int> path = {start};
        std::unordered_set<int> visited = {start};
        int current = start;

        const size_t MAX_STEPS = 5000;

        for (size_t step = 0; step < MAX_STEPS && current != end; ++step)
        {
            auto neighbors = graph->getNeighbors(current);
            if (neighbors.empty())
                break;

            // ищем соседа который приближает к цели
            int best_neighbor = -1;
            double best_score = std::numeric_limits<double>::max();

            for (int neighbor : neighbors)
            {
                if (visited.find(neighbor) != visited.end())
                    continue;

                try
                {
                    double cost = graph->getEdgeWeight(current, neighbor, strategy);
                    // простая эвристика - минимизируем стоимость
                    if (cost < best_score)
                    {
                        best_score = cost;
                        best_neighbor = neighbor;
                    }
                }
                catch (...)
                {
                    continue;
                }
            }

            if (best_neighbor == -1)
            {
                // все соседи посещены - выбираем случайного
                std::vector<int> unvisited;
                for (int neighbor : neighbors)
                {
                    if (visited.find(neighbor) == visited.end())
                    {
                        unvisited.push_back(neighbor);
                    }
                }
                if (unvisited.empty())
                    break;
                std::uniform_int_distribution<size_t> dist(0, unvisited.size() - 1);
                best_neighbor = unvisited[dist(gen)];
            }

            path.push_back(best_neighbor);
            visited.insert(best_neighbor);
            current = best_neighbor;
        }

        return (current == end) ? path : std::vector<int>();
    }

    bool GeneticAlgorithm::isValidPath(const std::vector<int> &path, const Domain::NetworkGraphPtr &graph,
                                       int start, int end)
    {
        if (path.size() < 2)
            return false;
        if (path[0] != start || path.back() != end)
            return false;

        for (size_t i = 0; i < path.size() - 1; ++i)
        {
            if (!graph->hasNode(path[i]) || !graph->hasNode(path[i + 1]))
            {
                return false;
            }
            auto neighbors = graph->getNeighbors(path[i]);
            if (std::find(neighbors.begin(), neighbors.end(), path[i + 1]) == neighbors.end())
            {
                return false;
            }
        }
        return true;
    }

    double GeneticAlgorithm::calculatePathCost(const std::vector<int> &path, const Domain::NetworkGraphPtr &graph)
    {
        double total_cost = 0.0;
        for (size_t i = 0; i + 1 < path.size(); ++i)
        {
            try
            {
                total_cost += graph->getEdgeWeight(path[i], path[i + 1], strategy);
            }
            catch (...)
            {
                return std::numeric_limits<double>::max();
            }
        }
        return total_cost;
    }
}
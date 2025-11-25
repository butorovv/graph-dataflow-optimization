#ifndef GENETICALGORITHM_H
#define GENETICALGORITHM_H

#include "IHeuristicSolver.h"
#include "../domain/WeightCalculator.h"
#include <random>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <queue>
#include <unordered_map>

namespace Infrastructure
{
    class GeneticAlgorithm : public IHeuristicSolver
    {
    public:
        GeneticAlgorithm(size_t population_size = 50,
                        size_t generations = 100,
                        double mutation_rate = 0.15,
                        double crossover_rate = 0.8,
                        Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::BALANCE_LOAD)
            : populationSize(population_size), generations(generations),
              mutationRate(mutation_rate), crossoverRate(crossover_rate), strategy(strategy),
              gen(std::random_device{}()) {}

        HeuristicResult optimize(const Domain::NetworkGraphPtr &graph,
                                 const std::vector<std::pair<int, int>> &demands) override;

    private:
        struct Chromosome {
            std::vector<int> path;
            double fitness;
            bool valid;
            
            bool operator<(const Chromosome& other) const {
                return fitness < other.fitness;
            }
        };

        size_t populationSize;
        size_t generations;
        double mutationRate;
        double crossoverRate;
        Domain::WeightCalculator::Strategy strategy;
        std::mt19937 gen;

        // генетические операторы
        std::vector<Chromosome> initializePopulation(const Domain::NetworkGraphPtr &graph, 
                                                   int start, int end);
        double calculateFitness(const Chromosome &chromosome, const Domain::NetworkGraphPtr &graph, int start, int end);
        Chromosome crossover(const Chromosome &parent1, const Chromosome &parent2, const Domain::NetworkGraphPtr &graph, int start, int end);
        void mutate(Chromosome &chromosome, const Domain::NetworkGraphPtr &graph, int start, int end);
        
        // методы генерации путей
        std::vector<int> generateRandomPathBFS(const Domain::NetworkGraphPtr &graph, int start, int end);
        std::vector<int> generateRandomPathDFS(const Domain::NetworkGraphPtr &graph, int start, int end);
        std::vector<int> generateGreedyPath(const Domain::NetworkGraphPtr &graph, int start, int end);
        
        // Вспомогательные методы
        bool isValidPath(const std::vector<int>& path, const Domain::NetworkGraphPtr &graph, int start, int end);
        double calculatePathCost(const std::vector<int>& path, const Domain::NetworkGraphPtr &graph);
    };
}

#endif
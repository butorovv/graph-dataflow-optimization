#ifndef GENETICALGORITHM_H
#define GENETICALGORITHM_H

#include "IHeuristicSolver.h"
#include "../domain/WeightCalculator.h"
#include <random>
#include <algorithm>

namespace Infrastructure
{
    class GeneticAlgorithm : public IHeuristicSolver
    {
    public:
        GeneticAlgorithm(size_t population_size = 100, 
                        size_t generations = 1000,
                        double mutation_rate = 0.1,
                        Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::BALANCE_LOAD)
            : populationSize(population_size), generations(generations),
              mutationRate(mutation_rate), strategy(strategy) {}

        HeuristicResult optimize(const Domain::NetworkGraphPtr &graph,
                                 const std::vector<std::pair<int, int>> &demands) override;

    private:
        struct Chromosome {
            std::vector<int> path;
            double fitness;
            
            bool operator<(const Chromosome& other) const {
                return fitness < other.fitness;
            }
        };

        size_t populationSize;
        size_t generations;
        double mutationRate;
        Domain::WeightCalculator::Strategy strategy;
        std::random_device rd;
        std::mt19937 gen{rd()};

        // Генетические операторы
        std::vector<Chromosome> initializePopulation(const Domain::NetworkGraphPtr &graph, 
                                                   int start, int end);
        double calculateFitness(const Chromosome &chromosome, const Domain::NetworkGraphPtr &graph);
        Chromosome crossover(const Chromosome &parent1, const Chromosome &parent2);
        void mutate(Chromosome &chromosome, const Domain::NetworkGraphPtr &graph);
        std::vector<int> generateRandomPath(const Domain::NetworkGraphPtr &graph, int start, int end);
    };
}

#endif
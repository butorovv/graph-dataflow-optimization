#ifndef ANTCOLONYOPTIMIZER_H
#define ANTCOLONYOPTIMIZER_H

#include "IHeuristicSolver.h"
#include "../domain/WeightCalculator.h"
#include <random>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Infrastructure
{
    class AntColonyOptimizer : public IHeuristicSolver
    {
    public:
        AntColonyOptimizer(size_t ant_count = 50,
                          size_t iterations = 100,
                          double alpha = 1.0,    // влияние феромона
                          double beta = 2.0,     // влияние эвристики
                          double evaporation = 0.5,
                          double Q = 100.0,      // константа феромона
                          Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::BALANCE_LOAD)
            : antCount(ant_count), iterations(iterations),
              alpha(alpha), beta(beta), evaporation(evaporation), Q(Q),
              strategy(strategy), gen(std::random_device{}()) {}

        HeuristicResult optimize(const Domain::NetworkGraphPtr &graph,
                                const std::vector<std::pair<int, int>> &demands) override;

    private:
        struct Ant {
            std::vector<int> path;
            std::unordered_set<int> visited;
            double path_cost;
        };

        size_t antCount;
        size_t iterations;
        double alpha;
        double beta;
        double evaporation;
        double Q;
        Domain::WeightCalculator::Strategy strategy;
        std::mt19937 gen;

        // матрицы феромонов и эвристической информации
        std::unordered_map<int, std::unordered_map<int, double>> pheromone;
        std::unordered_map<int, std::unordered_map<int, double>> heuristic;

        // основные методы муравьиной колонии
        void initializePheromones(const Domain::NetworkGraphPtr &graph);
        void initializeHeuristics(const Domain::NetworkGraphPtr &graph);
        Ant constructSolution(const Domain::NetworkGraphPtr &graph, int start, int end);
        int selectNextNode(const Ant& ant, const Domain::NetworkGraphPtr &graph, int end);
        void updatePheromones(std::vector<Ant>& ants);
        double calculateTransitionProbability(const Ant& ant, int current, int next, const Domain::NetworkGraphPtr &graph);
        
        // вспомогательные методы
        std::vector<int> getAvailableNeighbors(const Ant& ant, const Domain::NetworkGraphPtr &graph, int current);
        bool isValidPath(const std::vector<int>& path, const Domain::NetworkGraphPtr &graph);
        double calculatePathCost(const std::vector<int>& path, const Domain::NetworkGraphPtr &graph);
    };
}

#endif
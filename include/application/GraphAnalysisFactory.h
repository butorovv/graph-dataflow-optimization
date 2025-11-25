#ifndef GRAPHANALYSISFACTORY_H
#define GRAPHANALYSISFACTORY_H

#include "../domain/NetworkGraph.h"
#include "../infrastructure/IPathFinder.h"
#include "../infrastructure/IFlowSolver.h"
#include "../infrastructure/IHeuristicSolver.h"
#include "../domain/IGraphRepository.h"
#include <memory>

namespace Application
{
    class GraphAnalysisFactory
    {
    public:
        // репозитории
        static std::unique_ptr<Domain::IGraphRepository> createRepository();

        // алгоритмы поиска пути
        static Infrastructure::IPathFinderPtr createPathFinder();
        static Infrastructure::IPathFinderPtr createBGLPathFinder(
            bool useWeights = true,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY);
        static Infrastructure::IPathFinderPtr createAStarPathFinder(
            bool useWeights = true,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::MINIMIZE_LATENCY);
        static Infrastructure::IHeuristicSolverPtr createAntColonyOptimizer(
            size_t ant_count = 50,
            size_t iterations = 100,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::BALANCE_LOAD);
        // потоковые алгоритмы
        static Infrastructure::IFlowSolverPtr createFlowSolver();

        // эвристические алгоритмы
        static Infrastructure::IHeuristicSolverPtr createGeneticAlgorithm(
            size_t population_size = 100,
            Domain::WeightCalculator::Strategy strategy = Domain::WeightCalculator::BALANCE_LOAD);
    };
}

#endif
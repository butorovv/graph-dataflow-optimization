#include "application/GraphAnalysisFactory.h"
#include "infrastructure/FileGraphRepository.h"
#include "infrastructure/BGLShortestPath.h"
#include "infrastructure/AStarPathFinder.h"
#include "infrastructure/BoostFlowSolver.h"
#include "infrastructure/GeneticAlgorithm.h"
#include "infrastructure/AntColonyOptimizer.h"

namespace Application
{
    std::unique_ptr<Domain::IGraphRepository> GraphAnalysisFactory::createRepository()
    {
        return std::make_unique<Infrastructure::FileGraphRepository>();
    }

    Infrastructure::IPathFinderPtr GraphAnalysisFactory::createPathFinder()
    {
        return createAStarPathFinder(true, Domain::WeightCalculator::BALANCE_LOAD);
    }

    Infrastructure::IPathFinderPtr GraphAnalysisFactory::createBGLPathFinder(
        bool useWeights, Domain::WeightCalculator::Strategy strategy)
    {
        return std::make_unique<Infrastructure::BGLShortestPath>(useWeights, strategy);
    }

    Infrastructure::IPathFinderPtr GraphAnalysisFactory::createAStarPathFinder(
        bool useWeights, Domain::WeightCalculator::Strategy strategy)
    {
        return std::make_unique<Infrastructure::AStarPathFinder>(useWeights, strategy);
    }

    Infrastructure::IHeuristicSolverPtr GraphAnalysisFactory::createAntColonyOptimizer(
        size_t ant_count, size_t iterations, Domain::WeightCalculator::Strategy strategy)
    {
        return std::make_unique<Infrastructure::AntColonyOptimizer>(
            ant_count, iterations, 1.0, 2.0, 0.5, 100.0, strategy);
    }

    Infrastructure::IFlowSolverPtr GraphAnalysisFactory::createFlowSolver()
    {
        return std::make_unique<Infrastructure::BoostFlowSolver>();
    }

    Infrastructure::IHeuristicSolverPtr GraphAnalysisFactory::createGeneticAlgorithm(
        size_t population_size, Domain::WeightCalculator::Strategy strategy)
    {
        return std::make_unique<Infrastructure::GeneticAlgorithm>(population_size, 1000, 0.1, strategy);
    }
}
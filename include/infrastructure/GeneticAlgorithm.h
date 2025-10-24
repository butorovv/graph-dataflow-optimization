#ifndef GENETICALGORITHM_H
#define GENETICALGORITHM_H

#include "IHeuristicSolver.h"

namespace Infrastructure
{

    class GeneticAlgorithm : public IHeuristicSolver
    {
    public:
        HeuristicResult optimize(const Domain::NetworkGraphPtr &graph,
                                 const std::vector<std::pair<int, int>> &demands) override;

    private:
        // параметры GA
        size_t populationSize = 100;
        size_t generations = 1000;
        double mutationRate = 0.1;
    };

}
#endif
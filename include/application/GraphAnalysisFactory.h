#ifndef GRAPHANALYSISFACTORY_H
#define GRAPHANALYSISFACTORY_H

#include "../domain/NetworkGraph.h"
#include "../infrastructure/IPathFinder.h"
#include "../domain/IGraphRepository.h"
#include <memory>

namespace Application
{
    class GraphAnalysisFactory
    {
    public:
        static std::unique_ptr<Domain::IGraphRepository> createRepository();
        static Infrastructure::IPathFinderPtr createPathFinder();
    };
}

#endif
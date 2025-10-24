#include "application/GraphAnalysisFactory.h"
#include "infrastructure/FileGraphRepository.h"
#include "infrastructure/BGLShortestPath.h"

namespace Application
{
    std::unique_ptr<Domain::IGraphRepository> GraphAnalysisFactory::createRepository()
    {
        return std::make_unique<Infrastructure::FileGraphRepository>();
    }

    Infrastructure::IPathFinderPtr GraphAnalysisFactory::createPathFinder()
    {
        return std::make_unique<Infrastructure::BGLShortestPath>();
    }

}
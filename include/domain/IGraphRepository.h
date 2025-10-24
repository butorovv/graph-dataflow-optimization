#ifndef IGRAPHREPOSITORY_H
#define IGRAPHREPOSITORY_H

#include "NetworkGraph.h"
#include <memory>
#include <string>

namespace Domain
{
    class IGraphRepository
    {
    public:
        virtual ~IGraphRepository() = default;
        virtual NetworkGraphPtr loadGraph(const std::string &source) = 0;
    };

    using IGraphRepositoryPtr = std::unique_ptr<IGraphRepository>;

}

#endif
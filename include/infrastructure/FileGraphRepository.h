#ifndef FILEGRAPHREPOSITORY_H
#define FILEGRAPHREPOSITORY_H

#include "../domain/NetworkGraph.h"
#include "../domain/IGraphRepository.h"
#include <string>
#include <memory>

namespace Infrastructure
{
    class FileGraphRepository : public Domain::IGraphRepository
    {
    public:
        FileGraphRepository() = default;
        Domain::NetworkGraphPtr loadGraph(const std::string &filename) override;
    };

}

#endif
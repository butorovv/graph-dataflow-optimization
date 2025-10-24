#include "domain/NetworkGraph.h"
#include <sstream>
#include <stdexcept>

namespace Domain
{

    NetworkGraph::NetworkGraph(const std::string &name) : name(name) {}

    void NetworkGraph::addNode(int id, const std::string &)
    {
        nodes.insert(id);
        adjacency.try_emplace(id, std::unordered_map<int, double>{});
    }

    void NetworkGraph::addEdge(int source, int target, double weight)
    {
        addNode(source);
        addNode(target);
        adjacency[source][target] = weight;
    }

    bool NetworkGraph::hasNode(int id) const
    {
        return nodes.find(id) != nodes.end();
    }

    std::vector<int> NetworkGraph::getNeighbors(int node_id) const
    {
        std::vector<int> res;
        auto it = adjacency.find(node_id);
        if (it == adjacency.end())
            return res;
        res.reserve(it->second.size());
        for (const auto &kv : it->second)
            res.push_back(kv.first);
        return res;
    }

    double NetworkGraph::getEdgeWeight(int source, int target) const
    {
        auto it = adjacency.find(source);
        if (it == adjacency.end())
            throw std::out_of_range("Source node not found");
        auto it2 = it->second.find(target);
        if (it2 == it->second.end())
            throw std::out_of_range("Edge not found");
        return it2->second;
    }

    int NetworkGraph::getNodeCount() const
    {
        return static_cast<int>(nodes.size());
    }

    std::string NetworkGraph::getInfo() const
    {
        std::ostringstream oss;
        oss << name << " (nodes=" << getNodeCount() << ")";
        return oss.str();
    }

    std::vector<int> NetworkGraph::getAllNodeIds() const
    {
        std::vector<int> res;
        res.reserve(nodes.size());
        for (int id : nodes)
            res.push_back(id);
        return res;
    }

}
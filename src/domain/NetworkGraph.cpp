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

    void NetworkGraph::ensureNode(int id)
    {
        if (!hasNode(id)) addNode(id);
    }

    bool NetworkGraph::hasNode(int id) const
    {
        return nodes.find(id) != nodes.end();
    }

    bool NetworkGraph::removeEdge(int from, int to)
    {
        auto it = adjacency.find(from);
        if (it == adjacency.end()) return false;
        auto it2 = it->second.find(to);
        if (it2 == it->second.end()) return false;
        it->second.erase(it2);
        // если у from больше нет исходящих, можно удалить запись adjacency[from]
        if (it->second.empty()) adjacency.erase(it);
        // но node всё ещё считается существующим (nodes содержит id) — удалять узел не будем
        return true;
    }

    bool NetworkGraph::removeNode(int id)
    {
        // удалить все исходящие ребра
        adjacency.erase(id);
        // удалить все входящие ребра
        for (auto &kv : adjacency) {
            kv.second.erase(id);
        }
        // удалить сам узел
        auto it = nodes.find(id);
        if (it == nodes.end()) return false;
        nodes.erase(it);
        return true;
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

    NetworkGraphPtr NetworkGraph::createSnapshot() const
    {
        auto snap = std::make_shared<NetworkGraph>(this->name);
        snap->nodes = this->nodes;
        snap->adjacency = this->adjacency;
        return snap;
    }

}
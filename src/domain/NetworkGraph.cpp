#include "domain/NetworkGraph.h"
#include <sstream>
#include <stdexcept>

namespace Domain
{
    NetworkGraph::NetworkGraph(const std::string &name) : name(name) {}

    void NetworkGraph::addNode(int id, const std::string &) {
        nodes.insert(id);
        adjacency.try_emplace(id, std::unordered_map<int, LinkParameters>{});
    }

    void NetworkGraph::ensureNode(int id) {
        if (!hasNode(id)) addNode(id);
    }

    // версия с простым весом (для совместимости)
    void NetworkGraph::addEdge(int source, int target, double weight) {
        LinkParameters params;
        params.latency = weight;
        params.cost = weight;
        addEdge(source, target, params);
    }

    // версия с полными параметрами
    void NetworkGraph::addEdge(int source, int target, const LinkParameters &params) {
        addNode(source);
        addNode(target);
        adjacency[source][target] = params;
    }

    bool NetworkGraph::hasNode(int id) const {
        return nodes.find(id) != nodes.end();
    }

    std::vector<int> NetworkGraph::getNeighbors(int node_id) const {
        std::vector<int> res;
        auto it = adjacency.find(node_id);
        if (it == adjacency.end()) return res;
        
        res.reserve(it->second.size());
        for (const auto &kv : it->second)
            res.push_back(kv.first);
        return res;
    }

    // старая версия (для совместимости)
    double NetworkGraph::getEdgeWeight(int source, int target) const {
        return getEdgeParameters(source, target).latency;
    }

    // новая версия с стратегией агрегации
    double NetworkGraph::getEdgeWeight(int source, int target, WeightCalculator::Strategy strategy) const {
        auto params = getEdgeParameters(source, target);
        return WeightCalculator::calculateCompositeWeight(params, strategy);
    }

    LinkParameters NetworkGraph::getEdgeParameters(int source, int target) const {
        auto it = adjacency.find(source);
        if (it == adjacency.end())
            throw std::out_of_range("Source node not found");
        auto it2 = it->second.find(target);
        if (it2 == it->second.end())
            throw std::out_of_range("Edge not found");
        return it2->second;
    }

    int NetworkGraph::getNodeCount() const {
        return static_cast<int>(nodes.size());
    }

    std::string NetworkGraph::getInfo() const {
        std::ostringstream oss;
        oss << name << " (nodes=" << getNodeCount() << ")";
        return oss.str();
    }

    std::vector<int> NetworkGraph::getAllNodeIds() const {
        std::vector<int> res;
        res.reserve(nodes.size());
        for (int id : nodes) res.push_back(id);
        return res;
    }

    bool NetworkGraph::removeNode(int id) {
        adjacency.erase(id);
        for (auto &kv : adjacency) {
            kv.second.erase(id);
        }
        auto it = nodes.find(id);
        if (it == nodes.end()) return false;
        nodes.erase(it);
        return true;
    }

    bool NetworkGraph::removeEdge(int from, int to) {
        auto it = adjacency.find(from);
        if (it == adjacency.end()) return false;
        auto it2 = it->second.find(to);
        if (it2 == it->second.end()) return false;
        it->second.erase(it2);
        if (it->second.empty()) adjacency.erase(it);
        return true;
    }

    NetworkGraphPtr NetworkGraph::createSnapshot() const {
        auto snap = std::make_shared<NetworkGraph>(this->name);
        snap->nodes = this->nodes;
        snap->adjacency = this->adjacency;
        return snap;
    }
}
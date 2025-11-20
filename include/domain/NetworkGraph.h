#ifndef NETWORKGRAPH_H
#define NETWORKGRAPH_H

#include "NetworkTypes.h"
#include "WeightCalculator.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

namespace Domain
{

    class NetworkGraph
    {
    public:
        using AdjMap = std::unordered_map<int, std::unordered_map<int, double>>;

        NetworkGraph(const std::string &name = "network");

        // базовые операции с узлами и ребрами
        void addNode(int id, const std::string &name = "");
        void ensureNode(int id);

        // две версии добавления ребер - с параметрами и с простым весом
        void addEdge(int source, int target, const LinkParameters &params);
        void addEdge(int source, int target, double weight = 1.0);

        // операции с графом
        bool hasNode(int id) const;
        std::vector<int> getNeighbors(int node_id) const;
        double getEdgeWeight(int source, int target) const;
        double getEdgeWeight(int source, int target, WeightCalculator::Strategy strategy) const; // новая версия
        LinkParameters getEdgeParameters(int source, int target) const;

        // информация о графе
        int getNodeCount() const;
        std::string getInfo() const;
        std::vector<int> getAllNodeIds() const;
        bool removeNode(int id);
        bool removeEdge(int from, int to);
        NetworkGraphPtr createSnapshot() const; // для многопоточности

    private:
        std::string name;
        AdjMap adjacency;
        std::unordered_set<int> nodes;
    };

    // using NetworkGraphPtr = std::shared_ptr<NetworkGraph>; NetworkTypes.h уже даёт этот alias
}

#endif
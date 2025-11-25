#ifndef BASICMONITOR_H
#define BASICMONITOR_H

#include "../domain/NetworkGraph.h"
#include "../infrastructure/IPathFinder.h"
#include <iostream>
#include <vector>
#include <memory>

namespace Infrastructure
{
    class BasicMonitor
    {
    public:
        struct NetworkMetrics {
            int node_count;
            int edge_count;
            double average_degree;
            double density;
            double diameter_estimate;
        };
        
        static NetworkMetrics collectBasicMetrics(const Domain::NetworkGraphPtr& graph);
        static void simulateNodeFailure(const Domain::NetworkGraphPtr& graph, int node_id);
        static void simulateEdgeFailure(const Domain::NetworkGraphPtr& graph, int from, int to);
        static void analyzeConnectivity(const Domain::NetworkGraphPtr& graph, 
                                      const std::vector<std::pair<int, int>>& test_pairs);
    };
}

#endif
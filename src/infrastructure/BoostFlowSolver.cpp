#include "infrastructure/BoostFlowSolver.h"
#include "domain/NetworkGraph.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/graph/successive_shortest_path_nonnegative_weights.hpp>
#include <boost/graph/find_flow_cost.hpp>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <stdexcept>

namespace Infrastructure
{

    using Clock = std::chrono::steady_clock;
    using BGLGraph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::directedS,
        boost::no_property,
        boost::property<boost::edge_capacity_t, long,
                        boost::property<boost::edge_residual_capacity_t, long,
                                        boost::property<boost::edge_reverse_t,
                                                        boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>>::edge_descriptor,
                                                        boost::property<boost::edge_weight_t, long>>>>>;

    using EdgeDesc = boost::graph_traits<BGLGraph>::edge_descriptor;

    FlowResult BoostFlowSolver::solveMaxFlow(const Domain::NetworkGraphPtr &graph,
                                             int source, int sink)
    {
        FlowResult result;
        if (!graph || !graph->hasNode(source) || !graph->hasNode(sink))
        {
            result.success = false;
            result.errorMessage = "Source or sink node not found";
            return result;
        }

        auto start = Clock::now();

        std::vector<int> node_ids = graph->getAllNodeIds();
        std::unordered_map<int, int> node_to_idx;
        for (size_t i = 0; i < node_ids.size(); ++i)
            node_to_idx[node_ids[i]] = static_cast<int>(i);

        BGLGraph g(node_ids.size());
        auto capacity_map = get(boost::edge_capacity, g);
        auto reverse_map = get(boost::edge_reverse, g);

        for (int u_id : node_ids)
        {
            int u_idx = node_to_idx[u_id];
            for (int v_id : graph->getNeighbors(u_id))
            {
                if (node_to_idx.find(v_id) == node_to_idx.end())
                    continue;
                int v_idx = node_to_idx[v_id];

                EdgeDesc e, rev_e;
                bool inserted_e, inserted_rev;
                boost::tie(e, inserted_e) = boost::add_edge(u_idx, v_idx, g);
                boost::tie(rev_e, inserted_rev) = boost::add_edge(v_idx, u_idx, g);

                long cap = 1;
                try
                {
                    cap = static_cast<long>(graph->getEdgeWeight(u_id, v_id));
                }
                catch (...)
                {
                }

                capacity_map[e] = cap;
                capacity_map[rev_e] = 0;
                reverse_map[e] = rev_e;
                reverse_map[rev_e] = e;
            }
        }

        long flow = 0;
        try
        {
            flow = boost::push_relabel_max_flow(g, node_to_idx[source], node_to_idx[sink]);
        }
        catch (const std::exception &ex)
        {
            result.success = false;
            result.errorMessage = std::string("MaxFlow error: ") + ex.what();
            return result;
        }

        auto residual_map = get(boost::edge_residual_capacity, g);
        for (int u_id : node_ids)
        {
            int u = node_to_idx[u_id];
            for (int v_id : graph->getNeighbors(u_id))
            {
                int v = node_to_idx[v_id];
                EdgeDesc e;
                bool found;
                boost::tie(e, found) = edge(u, v, g);
                if (!found)
                    continue;
                long f = capacity_map[e] - residual_map[e];
                if (f > 0)
                    result.flowPerEdge[{u_id, v_id}] = static_cast<double>(f);
            }
        }

        result.maxFlow = static_cast<double>(flow);
        result.algorithmName = "Boost Push-Relabel";
        result.totalCost = static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count());
        result.success = true;
        return result;
    }

    FlowResult BoostFlowSolver::solveMinCostMaxFlow(const Domain::NetworkGraphPtr &graph,
                                                    int source, int sink)
    {
        FlowResult res;
        if (!graph || !graph->hasNode(source) || !graph->hasNode(sink))
        {
            res.success = false;
            res.errorMessage = "Source or sink not found";
            return res;
        }

        auto start = Clock::now();
        std::vector<int> node_ids = graph->getAllNodeIds();
        std::unordered_map<int, int> node_to_idx;
        for (size_t i = 0; i < node_ids.size(); ++i)
            node_to_idx[node_ids[i]] = static_cast<int>(i);

        BGLGraph g(node_ids.size());
        auto capacity = get(boost::edge_capacity, g);
        auto rev = get(boost::edge_reverse, g);
        auto weight = get(boost::edge_weight, g);

        for (int u_id : node_ids)
        {
            for (int v_id : graph->getNeighbors(u_id))
            {
                long cap = 1, cost = 0;
                try
                {
                    cap = static_cast<long>(graph->getEdgeWeight(u_id, v_id));
                }
                catch (...)
                {
                }

                EdgeDesc e1, e2;
                boost::tie(e1, std::ignore) = boost::add_edge(node_to_idx[u_id], node_to_idx[v_id], g);
                boost::tie(e2, std::ignore) = boost::add_edge(node_to_idx[v_id], node_to_idx[u_id], g);

                capacity[e1] = cap;
                capacity[e2] = 0;
                weight[e1] = cost;
                weight[e2] = -cost;
                rev[e1] = e2;
                rev[e2] = e1;
            }
        }

        try
        {
            boost::successive_shortest_path_nonnegative_weights(g, node_to_idx[source], node_to_idx[sink]);
        }
        catch (const std::exception &ex)
        {
            res.success = false;
            res.errorMessage = std::string("MinCostFlow error: ") + ex.what();
            return res;
        }

        long flow = 0;
        long total_flow_cost = boost::find_flow_cost(g);
        auto residual = get(boost::edge_residual_capacity, g);

        for (int u_id : node_ids)
        {
            for (int v_id : graph->getNeighbors(u_id))
            {
                int u = node_to_idx[u_id], v = node_to_idx[v_id];
                EdgeDesc e;
                bool found;
                boost::tie(e, found) = edge(u, v, g);
                if (!found)
                    continue;
                long f = capacity[e] - residual[e];
                if (f != 0)
                    res.flowPerEdge[{u_id, v_id}] = static_cast<double>(f);
            }
        }

        for (const auto &kv : res.flowPerEdge)
        {
            if (kv.first.first == source)
                flow += static_cast<long>(kv.second);
        }

        res.maxFlow = static_cast<double>(flow);
        res.flowCost = static_cast<double>(total_flow_cost);
        res.algorithmName = "Boost MinCostFlow (successive_shortest_path)";
        res.totalCost = static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start).count());
        res.success = true;
        return res;
    }

}

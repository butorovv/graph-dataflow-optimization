#include "infrastructure/SimpleStorage.h"
#include <iostream>

namespace Infrastructure
{
    void SimpleStorage::saveExperimentResults(const std::string& filename, 
                                            const std::vector<AlgorithmComparison>& results)
    {
        std::ofstream file(filename);
        file << "Algorithm,Success,Cost,Length,Time(ms)\n";
        
        for (const auto& result : results) {
            file << result.algorithmName << ","
                 << (result.success ? "true" : "false") << ","
                 << result.pathCost << ","
                 << result.pathLength << ","
                 << result.executionTime << "\n";
        }
        
        std::cout << "Результаты эксперимента сохранены в " << filename << "\n";
    }
    
    void SimpleStorage::logEvent(const std::string& event)
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        
        std::ofstream logfile("experiment_log.txt", std::ios_base::app);
        logfile << "[" << ss.str() << "] " << event << "\n";
    }
    
    void SimpleStorage::saveGraphInfo(const Domain::NetworkGraphPtr& graph, const std::string& filename)
    {
        std::ofstream file(filename);
        file << "Graph Info: " << graph->getInfo() << "\n";
        file << "Node Count: " << graph->getNodeCount() << "\n";
        
        auto nodes = graph->getAllNodeIds();
        file << "Nodes: ";
        for (int node : nodes) {
            file << node << " ";
        }
        file << "\n";
        
        int edge_count = 0;
        for (int node : nodes) {
            edge_count += graph->getNeighbors(node).size();
        }
        file << "Edge Count: " << edge_count << "\n";
        
        std::cout << "Информация о графе сохранена в " << filename << "\n";
    }
    
    void SimpleStorage::saveComparisonTable(const std::vector<AlgorithmComparison>& results, 
                                          const std::string& filename)
    {
        std::ofstream file(filename);
        file << "Algorithm\tSuccess\tCost\tLength\tTime(ms)\n";
        
        for (const auto& result : results) {
            file << result.algorithmName << "\t"
                 << (result.success ? "true" : "false") << "\t"
                 << result.pathCost << "\t"
                 << result.pathLength << "\t"
                 << result.executionTime << "\n";
        }
        
        std::cout << "Таблица сравнения сохранена в " << filename << "\n";
    }
}
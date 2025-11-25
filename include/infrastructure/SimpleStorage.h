#ifndef SIMPLESTORAGE_H
#define SIMPLESTORAGE_H

#include "../domain/NetworkGraph.h"
#include "../infrastructure/AlgorithmComparator.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Infrastructure
{
    class SimpleStorage
    {
    public:
        static void saveExperimentResults(const std::string& filename, 
                                        const std::vector<AlgorithmComparison>& results);
        static void logEvent(const std::string& event);
        static void saveGraphInfo(const Domain::NetworkGraphPtr& graph, const std::string& filename);
        static void saveComparisonTable(const std::vector<AlgorithmComparison>& results, 
                                      const std::string& filename);
    };
}

#endif
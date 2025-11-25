#include "infrastructure/BasicMonitor.h"
#include "infrastructure/BGLShortestPath.h"
#include <algorithm>

namespace Infrastructure
{
    BasicMonitor::NetworkMetrics BasicMonitor::collectBasicMetrics(const Domain::NetworkGraphPtr& graph)
    {
        NetworkMetrics metrics;
        metrics.node_count = graph->getNodeCount();
        
        // ограничиваем вычисления для больших графов
        int sample_size = std::min(1000, metrics.node_count);
        auto node_ids = graph->getAllNodeIds();
        
        int total_edges = 0;
        int max_degree = 0;
        
        // выборочный расчет вместо полного
        for (int i = 0; i < sample_size; ++i) {
            int node = node_ids[i];
            int degree = graph->getNeighbors(node).size();
            total_edges += degree;
            max_degree = std::max(max_degree, degree);
        }
        
        // экстраполируем на весь граф
        double scale_factor = static_cast<double>(metrics.node_count) / sample_size;
        metrics.edge_count = static_cast<int>(total_edges * scale_factor);
        metrics.average_degree = static_cast<double>(total_edges) / sample_size;
        metrics.density = (2.0 * metrics.edge_count) / (metrics.node_count * (metrics.node_count - 1));
        metrics.diameter_estimate = std::min(10.0, static_cast<double>(max_degree) * 2);
        
        std::cout << "БАЗОВЫЕ МЕТРИКИ СЕТИ (ВЫБОРОЧНЫЕ)\n";
        std::cout << "Узлов: " << metrics.node_count << "\n";
        std::cout << "Ребер (оценка): " << metrics.edge_count << "\n";
        std::cout << "Средняя степень: " << metrics.average_degree << "\n";
        std::cout << "Плотность графа: " << metrics.density << "\n";
        std::cout << "Оценка диаметра: " << metrics.diameter_estimate << "\n";
        
        return metrics;
    }
    
    void BasicMonitor::simulateNodeFailure(const Domain::NetworkGraphPtr& graph, int node_id)
    {
        std::cout << "\nСИМУЛЯЦИЯ ОТКАЗА УЗЛА " << node_id << " \n";
        
        if (!graph->hasNode(node_id)) {
            std::cout << "FAIL Узел " << node_id << " не существует\n";
            return;
        }
        
        auto snapshot = graph->createSnapshot();
        int original_count = snapshot->getNodeCount();
        
        if (snapshot->removeNode(node_id)) {
            std::cout << "OK Узел " << node_id << " успешно отключен\n";
            std::cout << "Осталось узлов: " << snapshot->getNodeCount() << " (было: " << original_count << ")\n";
            
            // проверяем связность на нескольких тестовых маршрутах
            auto nodes = snapshot->getAllNodeIds();
            if (nodes.size() >= 2) {
                BGLShortestPath finder(false);
                auto result = finder.findShortestPath(snapshot, nodes[0], nodes[1]);
                std::cout << "Связность после отказа: " << (result.success ? "OK" : "FAIL") << "\n";
            }
        } else {
            std::cout << "FAIL Не удалось отключить узел " << node_id << "\n";
        }
    }
    
    void BasicMonitor::simulateEdgeFailure(const Domain::NetworkGraphPtr& graph, int from, int to)
    {
        std::cout << "\nСИМУЛЯЦИЯ ОТКАЗА РЕБРА " << from << " → " << to << " \n";
        
        if (!graph->hasNode(from) || !graph->hasNode(to)) {
            std::cout << "FAIL Узлы не существуют\n";
            return;
        }
        
        auto snapshot = graph->createSnapshot();
        
        if (snapshot->removeEdge(from, to)) {
            std::cout << "OK Ребро " << from << " → " << to << " успешно отключено\n";
            
            // проверяем связность
            BGLShortestPath finder(false);
            auto result = finder.findShortestPath(snapshot, from, to);
            std::cout << "Связность после отказа: " << (result.success ? "OK" : "FAIL") << "\n";
        } else {
            std::cout << "FAIL Не удалось отключить ребро\n";
        }
    }
    
    void BasicMonitor::analyzeConnectivity(const Domain::NetworkGraphPtr& graph, 
                                         const std::vector<std::pair<int, int>>& test_pairs)
    {
        std::cout << "\nАНАЛИЗ СВЯЗНОСТИ\n";
        BGLShortestPath finder(false);
        int successful_routes = 0;
        
        for (const auto& [start, end] : test_pairs) {
            if (graph->hasNode(start) && graph->hasNode(end)) {
                auto result = finder.findShortestPath(graph, start, end);
                std::cout << "Маршрут " << start << " → " << end << ": " 
                          << (result.success ? "OK" : "FAIL") << "\n";
                if (result.success) successful_routes++;
            }
        }
        
        std::cout << "Успешных маршрутов: " << successful_routes << "/" << test_pairs.size() 
                  << " (" << (successful_routes * 100.0 / test_pairs.size()) << "%)\n";
    }
}
#include "infrastructure/FileGraphRepository.h"
#include "domain/NetworkGraph.h"
#include "domain/WeightCalculator.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <windows.h>
#include <random>

namespace Infrastructure {

static std::string cp1251_to_utf8(const std::string& src) {
    if (src.empty()) return {};
    int wlen = MultiByteToWideChar(1251, 0, src.data(), static_cast<int>(src.size()), nullptr, 0);
    if (wlen == 0) return {};
    std::vector<wchar_t> wbuf(wlen);
    MultiByteToWideChar(1251, 0, src.data(), static_cast<int>(src.size()), wbuf.data(), wlen);
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, nullptr, 0, nullptr, nullptr);
    if (u8len == 0) return {};
    std::vector<char> u8buf(u8len);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, u8buf.data(), u8len, nullptr, nullptr);
    return std::string(u8buf.data(), u8len);
}

Domain::NetworkGraphPtr FileGraphRepository::loadGraph(const std::string& filename) {
    auto graph = std::make_shared<Domain::NetworkGraph>("Multi-Param Social Network");

    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::string raw;
    in.seekg(0, std::ios::end);
    raw.resize(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::ios::beg);
    in.read(&raw[0], static_cast<std::streamsize>(raw.size()));
    in.close();

    std::string text;
    bool isUtf8 = false;
    if (raw.size() >= 3 &&
        static_cast<unsigned char>(raw[0]) == 0xEF &&
        static_cast<unsigned char>(raw[1]) == 0xBB &&
        static_cast<unsigned char>(raw[2]) == 0xBF) {
        isUtf8 = true;
        text = raw.substr(3);
    } else {
        size_t high = 0;
        for (unsigned char c : raw) if (c >= 0x80) ++high;
        if (high > raw.size() / 20) {
            text = cp1251_to_utf8(raw);
        } else {
            text = raw;
        }
    }

    std::vector<std::string> lines;
    {
        std::string cur;
        for (size_t i = 0; i < text.size(); ++i) {
            char c = text[i];
            if (c == '\r') continue;
            if (c == '\n') {
                lines.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        if (!cur.empty()) lines.push_back(cur);
    }

    std::cout << "Показ первых строк файла (" << filename << "):\n";
    for (size_t i = 0; i < lines.size() && i < 10; ++i) {
        std::cout << (i+1) << ": " << lines[i] << "\n";
    }

    auto trim = [](std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    };

    // генератор случайных чисел для заполнения многопараметрических весов
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> util_dist(0.1, 0.8);    // загрузка канала 10-80%
    std::uniform_real_distribution<> loss_dist(0.0, 0.05);   // потери пакетов 0-5%
    std::uniform_real_distribution<> rel_dist(0.85, 0.99);   // надежность 85-99%
    std::uniform_real_distribution<> bw_dist(50.0, 500.0);   // пропускная способность 50-500 Мбит/с
    std::uniform_real_distribution<> cost_dist(0.5, 5.0);    // стоимость 0.5-5.0

    int edges_loaded = 0;
    int unparsable = 0;
    int max_node_id = 0;
    int line_no = 0;

    for (const auto& rawline : lines) {
        ++line_no;
        std::string line = rawline;
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#' || line[0] == '%' || line[0] == '/' ) continue;

        // нормализация разделителей
        for (char &c : line) if (c == '\t' || c == ',') c = ' ';

        std::istringstream iss(line);
        int u = 0, v = 0;
        double file_weight = 1.0;
        
        if (!(iss >> u >> v)) {
            ++unparsable;
            if (line_no <= 20) std::cerr << "Warning: cannot parse line " << line_no << ": " << rawline << "\n";
            continue;
        }
        
        // пытаемся прочитать вес из файла (если есть)
        if (!(iss >> file_weight)) {
            file_weight = 1.0; // значение по умолчанию
        }

        // создаем многопараметрическую структуру LinkParameters
        Domain::LinkParameters params;
        
        // базовые параметры из файла
        params.latency = file_weight;           // используем файловый вес как задержку
        params.cost = file_weight * cost_dist(gen); // стоимость пропорциональна задержке
        
        // дополнительные параметры (генерируем реалистичные значения)
        params.bandwidth = bw_dist(gen);        // пропускная способность
        params.packet_loss = loss_dist(gen);    // потери пакетов
        params.utilization = util_dist(gen);    // загрузка канала
        params.reliability = rel_dist(gen);     // надежность

        // используем новый API с LinkParameters
        graph->ensureNode(u);
        graph->ensureNode(v);
        graph->addEdge(u, v, params);  // ← используем перегруженную версию с LinkParameters

        ++edges_loaded;
        max_node_id = std::max(max_node_id, std::max(u, v));
    }

    std::cout << "Успешно загружен многопараметрический граф: " << graph->getNodeCount()
              << " узлов, " << edges_loaded << " связей\n";
    std::cout << "Максимальный ID узла: " << max_node_id << "\n";
    std::cout << "Параметры связей: latency, bandwidth, packet_loss, utilization, cost, reliability\n";
    
    if (unparsable > 0) {
        std::cout << "Нераспаршенных строк: " << unparsable << "\n";
    }

    // демонстрация многопараметрической системы для первых 3 ребер
    if (edges_loaded >= 3) {
        std::cout << "\nДЕМОНСТРАЦИЯ МНОГОПАРАМЕТРИЧЕСКИХ ВЕСОВ\n";
        
        // получаем список всех узлов и ребер для демонстрации
        auto node_ids = graph->getAllNodeIds();
        if (node_ids.size() >= 2) {
            int demo_node = node_ids[0];
            auto neighbors = graph->getNeighbors(demo_node);
            
            if (!neighbors.empty()) {
                int demo_target = neighbors[0];
                
                try {
                    auto params = graph->getEdgeParameters(demo_node, demo_target);
                    
                    std::cout << "Пример ребра " << demo_node << " → " << demo_target << ":\n";
                    std::cout << "  latency: " << params.latency << " ms\n";
                    std::cout << "  bandwidth: " << params.bandwidth << " Mbps\n";
                    std::cout << "  packet_loss: " << (params.packet_loss * 100) << "%\n";
                    std::cout << "  utilization: " << (params.utilization * 100) << "%\n";
                    std::cout << "  cost: " << params.cost << "\n";
                    std::cout << "  reliability: " << (params.reliability * 100) << "%\n";
                    
                    // демонстрация разных стратегий агрегации
                    std::cout << "\nАгрегированные веса по разным стратегиям:\n";
                    auto strategies = {
                        Domain::WeightCalculator::MINIMIZE_LATENCY,
                        Domain::WeightCalculator::BALANCE_LOAD,
                        Domain::WeightCalculator::MAXIMIZE_BANDWIDTH,
                        Domain::WeightCalculator::MINIMIZE_COST
                    };
                    
                    for (auto strategy : strategies) {
                        double weight = graph->getEdgeWeight(demo_node, demo_target, strategy);
                        std::cout << "Стратегия " << strategy << ": " << weight << "\n";
                    }
                    
                } catch (const std::exception& e) {
                    std::cout << "Ошибка получения параметров: " << e.what() << "\n";
                }
            }
        }
    }

    return graph;
}

} // namespace Infrastructure
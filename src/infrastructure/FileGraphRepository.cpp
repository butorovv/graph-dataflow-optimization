#include "infrastructure/FileGraphRepository.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace Infrastructure
{

    Domain::NetworkGraphPtr FileGraphRepository::loadGraph(const std::string &filename)
    {
        auto graph = std::make_shared<Domain::NetworkGraph>("Facebook Social Network");
        std::ifstream file(filename);

        if (!file.is_open())
        {
            throw std::runtime_error("FileGraphRepository::loadGraph: cannot open file: " + filename);
        }

        std::string line;
        int edges_loaded = 0;
        int max_node_id = 0;

        std::cout << "📥 Загрузка Facebook датасета..." << std::endl;

        while (std::getline(file, line))
        {
            // пропускаем пустые строки
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            int u, v;
            if (iss >> u >> v)
            {
                // обновляем максимальный ID
                max_node_id = std::max({max_node_id, u, v});
                // добавляем узлы (если они еще не добавлены)
                graph->addNode(u);
                graph->addNode(v);
                // добавляем ребро (неориентированное)
                graph->addEdge(u, v);
                edges_loaded++;
            }
        }

        file.close();
        std::cout << "✅ Успешно загружен Facebook граф: " << graph->getNodeCount()
                  << " пользователей, " << edges_loaded << " дружеских связей\n";
        std::cout << "📊 Максимальный ID пользователя: " << max_node_id << "\n";

        return graph;
    }
}
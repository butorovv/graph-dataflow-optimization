#ifndef NETWORKTYPES_H
#define NETWORKTYPES_H

#include <memory>
#include <vector>
#include <string>

namespace Domain
{

    class NetworkGraph;
    using NetworkGraphPtr = std::shared_ptr<NetworkGraph>;
    // результат поиска пути в графе
    struct PathResult
    {
        bool success = false;       // успешно ли найден путь
        std::string errorMessage;   // сообщение об ошибке, если путь не найден
        std::vector<int> pathNodes; // последовательность вершин в найденном пути
        double totalCost = 0.0;     // общая стоимость пути
        std::string algorithmName;  // название использованного алгоритма
        // конструктор по умолчанию для корректной работы с STL
        PathResult() = default;
    };
}

#endif
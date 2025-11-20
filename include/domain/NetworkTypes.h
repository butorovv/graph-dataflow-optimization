#ifndef NETWORKTYPES_H
#define NETWORKTYPES_H

#include <memory>
#include <vector>
#include <string>

namespace Domain
{
    class NetworkGraph;
    using NetworkGraphPtr = std::shared_ptr<NetworkGraph>;

    // многопараметрическая структура связи
    struct LinkParameters {
        double latency = 1.0;       // задержка (мс)
        double bandwidth = 1.0;     // пропускная способность (Мбит/с)
        double packet_loss = 0.0;   // потери пакетов (0.0-1.0)
        double utilization = 0.0;   // загрузка канала (0.0-1.0)
        double cost = 1.0;          // стоимость передачи
        double reliability = 1.0;   // надежность (0.0-1.0)
        LinkParameters() = default;
        LinkParameters(double weight) : latency(weight), cost(weight) {}
    };

    // результат поиска пути в графе
    struct PathResult
    {
        bool success = false;       // успешно ли найден путь
        std::string errorMessage;   // сообщение об ошибке, если путь не найден
        std::vector<int> pathNodes; // последовательность вершин в найденном пути
        double totalCost = 0.0;     // общая стоимость пути
        std::string algorithmName;  // название использованного алгоритма
        double executionTime = 0.0; // время выполнения в ms
        // конструктор по умолчанию для корректной работы с STL
        PathResult() = default;
    };
}

#endif
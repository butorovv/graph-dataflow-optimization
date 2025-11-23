# Оптимизация потоков данных в графах

Проект анализирует потоки данных в графах с использованием алгоритмов
оптимизации в распределённых системах управления. Включает вычисление
максимального потока и минимальной стоимости максимального потока на графах социальных сетей.

---

## Требования

- Windows
- TDM-GCC 64-bit
- Библиотеки Boost C++ (тестировалось на Boost 1.89.0)
- Git Bash (опционально, для выполнения команд ниже)

---

## Структура проекта

```
build/               -> Скомпилированные исполняемые файлы
data/datasets/       -> Входные данные (CSV-файлы)
include/             -> Заголовочные файлы (application, domain, infrastructure)
src/                 -> Исходники (application, domain, infrastructure)
src/main.cpp         -> Точка входа программы
```

---

## Сборка проекта

Откройте Git Bash или терминал с TDM-GCC в PATH и выполните команду из корня проекта:

```bash
g++ -std=c++17 -Iinclude -IC:/boost_1_89_0 -O2 -o build/graph_research.exe     
src/main.cpp     src/application/GraphAnalysisFactory.cpp     
src/application/GraphAnalysisService.cpp     src/domain/NetworkGraph.cpp     
src/domain/NetworkTypes.cpp     src/domain/WeightCalculator.cpp     
src/infrastructure/AlgorithmComparator.cpp     src/infrastructure/AStarPathFinder.cpp     
src/infrastructure/BasicMonitor.cpp     src/infrastructure/BGLShortestPath.cpp     
src/infrastructure/BoostFlowSolver.cpp     src/infrastructure/FileGraphRepository.cpp     
src/infrastructure/GeneticAlgorithm.cpp     src/infrastructure/SimpleStorage.cpp     
src/infrastructure/ThreadPool.cpp     src/infrastructure/AntColonyOptimizer.cpp
```

- `-Iinclude` — путь к заголовочным файлам проекта  
- `-IC:/boost_1_89_0` — путь к установленной библиотеке Boost  
- `-O2` — включение оптимизации  
- Исполняемый файл создаётся в `build/graph_research.exe`

---

## Запуск программы

После успешной сборки выполните:

```bash
./build/graph_research.exe
```

Программа загрузит данные из `data/datasets/` и выведет результаты анализа алгоритмов максимального потока и минимальной стоимости потока.

---

## Примечания

- Папка `.vscode/` не требуется для запуска.  
- Датасеты включены в репозиторий для воспроизводимости экспериментов.  
- Заголовочные файлы Boost должны совпадать с указанным в команде компиляции `-I` путём.  

---

## Автор

Георгий Буторов, группа ИУ1-52Б

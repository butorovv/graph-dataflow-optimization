#ifndef STRATEGYCONFIG_H
#define STRATEGYCONFIG_H

#include "domain/WeightCalculator.h"
#include <string>

namespace Config
{
    struct StrategySettings {
        // стратегии для разных типов алгоритмов
        Domain::WeightCalculator::Strategy exact_uniform = Domain::WeightCalculator::UNIFORM_WEIGHTS;
        Domain::WeightCalculator::Strategy exact_multi_param = Domain::WeightCalculator::BALANCE_LOAD;
        Domain::WeightCalculator::Strategy genetic = Domain::WeightCalculator::BALANCE_LOAD;
        Domain::WeightCalculator::Strategy ant_colony = Domain::WeightCalculator::BALANCE_LOAD;
        
        // пресеты для быстрого переключения между сценариями
        
        // 1) оптимизация задержки (для чувствительных к задержке приложений)
        static StrategySettings createLatencyOptimized() {
            return {
                Domain::WeightCalculator::UNIFORM_WEIGHTS,
                Domain::WeightCalculator::MINIMIZE_LATENCY,
                Domain::WeightCalculator::MINIMIZE_LATENCY,
                Domain::WeightCalculator::MINIMIZE_LATENCY
            };
        }
        
        // 2) оптимизация пропускной способности (для потокового видео, больших файлов)
        static StrategySettings createBandwidthOptimized() {
            return {
                Domain::WeightCalculator::UNIFORM_WEIGHTS,
                Domain::WeightCalculator::MAXIMIZE_BANDWIDTH,
                Domain::WeightCalculator::MAXIMIZE_BANDWIDTH,
                Domain::WeightCalculator::MAXIMIZE_BANDWIDTH
            };
        }
        
        // 3) балансировка нагрузки (универсальный сценарий)
        static StrategySettings createBalanced() {
            return {
                Domain::WeightCalculator::UNIFORM_WEIGHTS,
                Domain::WeightCalculator::BALANCE_LOAD,
                Domain::WeightCalculator::BALANCE_LOAD,
                Domain::WeightCalculator::BALANCE_LOAD
            };
        }
        
        // 4) минимизация стоимости (бюджетные решения)
        static StrategySettings createCostOptimized() {
            return {
                Domain::WeightCalculator::UNIFORM_WEIGHTS,
                Domain::WeightCalculator::MINIMIZE_COST,
                Domain::WeightCalculator::MINIMIZE_COST,
                Domain::WeightCalculator::MINIMIZE_COST
            };
        }
        
        // 5) адаптивная стратегия (интеллектуальное управление)
        static StrategySettings createAdaptive() {
            return {
                Domain::WeightCalculator::UNIFORM_WEIGHTS,
                Domain::WeightCalculator::ADAPTIVE_WEIGHTS,
                Domain::WeightCalculator::ADAPTIVE_WEIGHTS,
                Domain::WeightCalculator::ADAPTIVE_WEIGHTS
            };
        }
        
        // 6) смешанный сценарий (разные стратегии для разных алгоритмов)
        static StrategySettings createMixed() {
            return {
                Domain::WeightCalculator::UNIFORM_WEIGHTS,           // uniform - всегда одинаковый
                Domain::WeightCalculator::BALANCE_LOAD,              // multi-param - баланс
                Domain::WeightCalculator::MINIMIZE_LATENCY,          // genetic - минимум задержки
                Domain::WeightCalculator::MAXIMIZE_BANDWIDTH         // ant colony - максимум пропускной
            };
        }
        
        // получить описание конфигурации
        std::string getDescription() const {
            return "Config[Uniform: " + Domain::WeightCalculator::getStrategyName(exact_uniform) +
                   ", Multi-Param: " + Domain::WeightCalculator::getStrategyName(exact_multi_param) +
                   ", Genetic: " + Domain::WeightCalculator::getStrategyName(genetic) +
                   ", Ant Colony: " + Domain::WeightCalculator::getStrategyName(ant_colony) + "]";
        }
    };
}

#endif
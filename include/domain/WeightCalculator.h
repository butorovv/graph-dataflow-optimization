#ifndef WEIGHTCALCULATOR_H
#define WEIGHTCALCULATOR_H

#include "NetworkTypes.h"
#include <cmath>

namespace Domain
{
    class WeightCalculator
    {
    public:
        enum Strategy {
            UNIFORM_WEIGHTS,    // все веса = 1 (версия без весов)
            MINIMIZE_LATENCY,   // минимизация задержки
            BALANCE_LOAD,       // балансировка нагрузки с нелинейными зависимостями
            MAXIMIZE_BANDWIDTH, // максимизация пропускной способности
            MINIMIZE_COST,      // минимизация стоимости
            ADAPTIVE_WEIGHTS    // адаптивная стратегия
        };

        // основная функция агрегации параметров в скалярный вес
        static double calculateCompositeWeight(const LinkParameters& params, Strategy strategy);
        
        // анализ влияния параметров друг на друга
        static void analyzeParameterDependencies(const LinkParameters& params);

    private:
        // приватные методы для разных стратегий
        static double calculateUniformWeight();
        static double calculateLatencyWeight(const LinkParameters& params);
        static double calculateBalancedWeight(const LinkParameters& params);
        static double calculateBandwidthWeight(const LinkParameters& params);
        static double calculateCostWeight(const LinkParameters& params);
        static double calculateAdaptiveWeight(const LinkParameters& params);
        
        // функции для моделирования зависимостей между параметрами
        static double calculateEffectiveLatency(double base_latency, double utilization);
        static double calculateCurrentBandwidth(double max_bandwidth, double packet_loss);
        static double calculateDynamicReliability(double packet_loss, double utilization);
    };
}

#endif
#include "domain/WeightCalculator.h"
#include <iostream>

namespace Domain
{
    double WeightCalculator::calculateCompositeWeight(const LinkParameters& params, Strategy strategy)
    {
        switch(strategy) {
            case UNIFORM_WEIGHTS:    return calculateUniformWeight();
            case MINIMIZE_LATENCY:   return calculateLatencyWeight(params);
            case BALANCE_LOAD:       return calculateBalancedWeight(params);
            case MAXIMIZE_BANDWIDTH: return calculateBandwidthWeight(params);
            case MINIMIZE_COST:      return calculateCostWeight(params);
            case ADAPTIVE_WEIGHTS:   return calculateAdaptiveWeight(params);
            default:                 return calculateLatencyWeight(params);
        }
    }

    double WeightCalculator::calculateUniformWeight() {
        return 1.0; // версия без весов
    }

    double WeightCalculator::calculateLatencyWeight(const LinkParameters& params) {
        return params.latency; // простая минимизация задержки
    }

    double WeightCalculator::calculateBalancedWeight(const LinkParameters& params) {
        // нелинейные зависимости между параметрами
        double effective_latency = calculateEffectiveLatency(params.latency, params.utilization);
        double current_bandwidth = calculateCurrentBandwidth(params.bandwidth, params.packet_loss);
        double dynamic_reliability = calculateDynamicReliability(params.packet_loss, params.utilization);
        
        // композитный вес с учетом взаимовлияния параметров
        return effective_latency * 0.5 + 
               (1.0 / current_bandwidth) * 0.2 +
               params.cost * 0.15 +
               (1.0 - dynamic_reliability) * 0.15;
    }

    double WeightCalculator::calculateBandwidthWeight(const LinkParameters& params) {
        return 1.0 / params.bandwidth; // обратная пропускная способность
    }

    double WeightCalculator::calculateCostWeight(const LinkParameters& params) {
        return params.cost; // минимизация стоимости
    }

    double WeightCalculator::calculateAdaptiveWeight(const LinkParameters& params) {
        // адаптивная стратегия на основе текущих условий сети
        if (params.utilization > 0.8) {
            // при высокой загрузке надежность
            return (1.0 - params.reliability) * 100.0;
        } else if (params.packet_loss > 0.1) {
            // при высоких потерях пропускная способность
            return 1.0 / params.bandwidth;
        } else {
            // в нормальных условиях - сбалансированный подход
            return calculateBalancedWeight(params);
        }
    }

    // моделирование взаимовлияния параметров
    double WeightCalculator::calculateEffectiveLatency(double base_latency, double utilization) {
        // задержка растет нелинейно при высокой загрузке
        return base_latency * (1.0 + 3.0 * utilization * utilization);
    }

    double WeightCalculator::calculateCurrentBandwidth(double max_bandwidth, double packet_loss) {
        // эффективная пропускная способность уменьшается из-за потерь
        return max_bandwidth * (1.0 - packet_loss);
    }

    double WeightCalculator::calculateDynamicReliability(double packet_loss, double utilization) {
        // надежность падает при высоких потерях и загрузке
        double penalty = packet_loss * 0.7 + utilization * 0.3;
        return std::max(0.0, 1.0 - penalty);
    }

    void WeightCalculator::analyzeParameterDependencies(const LinkParameters& params) {
        std::cout << "АНАЛИЗ ВЛИЯНИЯ ПАРАМЕТРОВ\n";
        std::cout << "Базовые параметры: latency=" << params.latency 
                  << ", bandwidth=" << params.bandwidth 
                  << ", utilization=" << params.utilization << "\n";
        
        // анализ влияния utilization на effective_latency
        std::cout << "Влияние загрузки на задержку:\n";
        for (double util = 0.0; util <= 1.0; util += 0.2) {
            double eff_lat = calculateEffectiveLatency(params.latency, util);
            std::cout << "  Utilization " << util << " - Effective latency: " << eff_lat 
                      << " (увеличение в " << eff_lat/params.latency << " раз)\n";
        }
        
        // анализ влияния packet_loss на bandwidth
        std::cout << "Влияние потерь на пропускную способность:\n";
        for (double loss = 0.0; loss <= 0.5; loss += 0.1) {
            double curr_bw = calculateCurrentBandwidth(params.bandwidth, loss);
            std::cout << "  Packet loss " << loss << " - Current bandwidth: " << curr_bw 
                      << " (" << (curr_bw/params.bandwidth)*100 << "% от максимума)\n";
        }
    }
}
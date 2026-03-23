#pragma once

#include "calculation_weight.h"

#include <unordered_map>

struct CalculationFCM {
    std::unordered_map<size_t, double> concepts;
    std::unordered_map<size_t, CalculationWeight> weights;
    std::unordered_map<size_t, size_t> conceptsStartSteps;
    double metricValue;
};

#pragma once

#include "calculation_concept.h"
#include "calculation_weight.h"

#include <unordered_map>

struct CalculationFCM {
    std::unordered_map<size_t, CalculationConcept> concepts;
    std::unordered_map<size_t, CalculationWeight> weights;
    double metricValue;
};

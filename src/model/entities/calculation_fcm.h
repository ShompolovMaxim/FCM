#pragma once

#include "calculation_concept.h"
#include "calculation_weight.h"

#include <map>

struct CalculationFCM {
    std::map<QUuid, CalculationConcept> concepts;
    std::map<QUuid, CalculationWeight> weights;
    double metricValue;
};

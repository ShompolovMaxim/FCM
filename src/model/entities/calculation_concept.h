#pragma once

#include "triangular_fuzzy_value.h"

#include <cstdint>

struct CalculationConcept {
    size_t id;
    double value;
    TriangularFuzzyValue triangularFuzzyValue;
    size_t startStep;
};

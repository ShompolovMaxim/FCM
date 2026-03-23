#pragma once

#include <cstdint>

struct CalculationWeight {
    size_t id;
    double value;
    size_t fromConceptId;
    size_t toConceptId;
};

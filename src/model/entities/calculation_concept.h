#pragma once

#include "triangular_fuzzy_value.h"

#include <cstdint>

#include <QUuid>

struct CalculationConcept {
    QUuid id;
    double value;
    TriangularFuzzyValue triangularFuzzyValue;
    size_t startStep;
};

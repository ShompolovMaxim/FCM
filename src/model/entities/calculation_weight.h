#pragma once

#include "triangular_fuzzy_value.h"

#include <cstdint>

#include <QUuid>

struct CalculationWeight {
    QUuid id;
    double value;
    TriangularFuzzyValue triangularFuzzyValue;
    QUuid fromConceptId;
    QUuid toConceptId;
};

#pragma once

#include "model/entities/triangular_fuzzy_value.h"

class ActivationFunction {
public:
    virtual double activate(double value) const = 0;

    TriangularFuzzyValue activate(TriangularFuzzyValue value);

    virtual ~ActivationFunction() = default;
};

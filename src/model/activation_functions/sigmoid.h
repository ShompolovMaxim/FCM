#pragma once

#include "activation_function.h"

class Sigmoid : public ActivationFunction
{
public:
    Sigmoid(double fuzzinessDegree = 1);

    double activate(double value) const override;

private:
    double fuzzinessDegree;
};

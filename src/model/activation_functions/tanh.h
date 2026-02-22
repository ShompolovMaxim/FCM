#pragma once

#include "activation_function.h"

class Tanh : public ActivationFunction
{
public:
    Tanh(double fuzzinessDegree = 1);

    double activate(double value) const override;

private:
    double fuzzinessDegree;
};

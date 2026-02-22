#pragma once

#include "activation_function.h"

class Bivalent : public ActivationFunction
{
public:
    Bivalent(double min = 0, double max = 1);

    double activate(double value) const override;

private:
    double min;
    double max;
};

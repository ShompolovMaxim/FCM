#pragma once

#include "activation_function.h"

class ThresholdLinear : public ActivationFunction
{
public:
    ThresholdLinear(double min = 0, double max = 1);

    double activate(double value) const override;
private:
    double min;
    double max;
};

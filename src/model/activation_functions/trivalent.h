#pragma once

#include "activation_function.h"

class Trivalent : public ActivationFunction
{
public:
    Trivalent(double min = 0, double max = 1);

    double activate(double value) const override;
private:
    double min;
    double max;
};

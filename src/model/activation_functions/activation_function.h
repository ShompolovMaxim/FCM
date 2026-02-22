# pragma once

class ActivationFunction {
public:
    virtual double activate(double value) const = 0;

    virtual ~ActivationFunction() = default;
};

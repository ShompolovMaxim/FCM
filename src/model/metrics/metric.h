#pragma once
#include <vector>

class Metric {
public:
    virtual double calculate(const std::vector<double>& a, const std::vector<double>& b) = 0;

    virtual ~Metric() = default;
};

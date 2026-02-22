#pragma once
#include "metric.h"

class MSE : public Metric
{
public:
    double calculate(const std::vector<double>& a, const std::vector<double>& b) override;
};

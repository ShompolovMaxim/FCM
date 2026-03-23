#pragma once
#include "metric.h"

class MAE : public Metric
{
public:
    double calculate(const std::vector<double>& a, const std::vector<double>& b) override;
};

#pragma once
#include "metric.h"

class MAPE : public Metric
{
public:
    double calculate(const std::vector<double>& a, const std::vector<double>& b) override;
};

#pragma once

#include "entities/calculation_fcm.h"

#include "metrics/metric.h"

#include <memory>

class MetricsManager {
public:
    MetricsManager(std::shared_ptr<Metric> metric, bool weightsChange);

    double calculate(const CalculationFCM& oldFcm, const CalculationFCM& newFcm);

private:
    bool weightsChange;
    std::shared_ptr<Metric> metric;
};

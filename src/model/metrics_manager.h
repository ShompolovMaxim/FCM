#pragma once

#include "entities/calculation_fcm.h"

#include "metrics/metric.h"

#include "presenter/prediction_parameters.h"

#include <memory>

class MetricsManager {
public:
    MetricsManager(std::shared_ptr<Metric> metric, const PredictionParameters& predictionParameters);

    double calculate(const CalculationFCM& oldFcm, const CalculationFCM& newFcm);

private:
    std::shared_ptr<Metric> metric;
    const PredictionParameters predictionParameters;
};

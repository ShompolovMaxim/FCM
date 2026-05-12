#pragma once

#include "model/entities/calculation/calculation_fcm.h"

#include "model/metrics/metric.h"

#include "model/prediction/prediction_parameters.h"

#include <memory>

class MetricsManager {
public:
    MetricsManager(std::shared_ptr<Metric> metric, const PredictionParameters& predictionParameters);

    double calculate(const CalculationFCM& oldFcm, const CalculationFCM& newFcm);

private:
    std::shared_ptr<Metric> metric;
    const PredictionParameters predictionParameters;
};


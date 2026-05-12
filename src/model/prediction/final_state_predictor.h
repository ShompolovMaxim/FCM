#pragma once

#include "model/metrics/metrics_manager.h"

#include "model/algorithms/algorithm.h"
#include "model/entities/calculation/calculation_fcm.h"
#include "model/stop_conditions/stop_condition.h"

#include "model/prediction/prediction_parameters.h"

#include <atomic>

class FinalStatePredictor {
public:
    FinalStatePredictor(const PredictionParameters& predictionParameters);

    CalculationFCM predict(const CalculationFCM& fcm);
    void requestStop();

private:
    const PredictionParameters predictionParameters;
    std::shared_ptr<PredictionAlgorithm> algorithm;
    std::shared_ptr<StopCondition> stopCondition;
    std::shared_ptr<MetricsManager> metricsManager;
    std::vector<CalculationFCM> fcms;
    std::atomic_bool stopRequested = false;
};


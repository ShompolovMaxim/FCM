#pragma once

#include "model/entities/calculation/calculation_fcm.h"

#include "model/prediction/prediction_parameters.h"

#include <vector>

class StopCondition {
public:
    StopCondition(const PredictionParameters& predictionParameters) : predictionParameters(predictionParameters) {}

    virtual ~StopCondition() = default;

    virtual bool finished(const std::vector<CalculationFCM>& fcms) = 0;

protected:
    const PredictionParameters predictionParameters;
};


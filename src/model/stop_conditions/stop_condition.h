#pragma once

#include "model/entities/calculation_fcm.h"

#include "presenter/prediction_parameters.h"

#include <vector>

class StopCondition {
public:
    StopCondition(const PredictionParameters& predictionParameters) : predictionParameters(predictionParameters) {}

    virtual ~StopCondition() = default;

    virtual bool finished(const std::vector<CalculationFCM>& fcms) = 0;

protected:
    const PredictionParameters predictionParameters;
};

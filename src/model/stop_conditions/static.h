#pragma once

#include "stop_condition.h"

class StaticCondition : public StopCondition
{
public:
    StaticCondition(const PredictionParameters& predictionParameters);

    bool finished(const std::vector<CalculationFCM>& fcms) override;
};


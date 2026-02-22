#pragma once

#include "algorithm.h"

class StandardPredictionAlgorithm : public PredictionAlgorithm
{
public:
    StandardPredictionAlgorithm(std::shared_ptr<ActivationFunction> activationFunction);

    CalculationFCM step(const CalculationFCM& fcm) const override;
};

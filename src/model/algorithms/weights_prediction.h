#pragma once

#include "algorithm.h"

class WeightsPredictionAlgorithm : public PredictionAlgorithm
{
public:
    WeightsPredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction);

    CalculationFCM step(const CalculationFCM& fcm) const override;
};

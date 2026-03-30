#pragma once

#include "algorithm.h"

class WeightsPredictionFuzzyAlgorithm : public PredictionAlgorithm
{
public:
    WeightsPredictionFuzzyAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction);

    CalculationFCM step(const CalculationFCM& fcm) const override;
};


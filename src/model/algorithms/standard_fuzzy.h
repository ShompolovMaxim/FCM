#pragma once

#include "algorithm.h"

class StandardFuzzyAlgorithm : public PredictionAlgorithm
{
public:
    StandardFuzzyAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction);

    CalculationFCM step(const CalculationFCM& fcm) const override;
};


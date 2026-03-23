#pragma once

#include "algorithm.h"

class StandardPredictionAlgorithm : public PredictionAlgorithm
{
public:
    StandardPredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction);

    CalculationFCM step(const CalculationFCM& fcm) const override;
};

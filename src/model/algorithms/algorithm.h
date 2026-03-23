#pragma once

#include "model/activation_functions/activation_function.h"
#include "model/entities/calculation_fcm.h"

#include <memory>

class PredictionAlgorithm {
public:
    PredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
        conceptsActivationFunction(conceptsActivationFunction), weightsActivationFunction(weightsActivationFunction) {}

    virtual CalculationFCM step(const CalculationFCM& fcm) const = 0;

    virtual ~PredictionAlgorithm() = default;

protected:
    std::shared_ptr<ActivationFunction> conceptsActivationFunction = nullptr;
    std::shared_ptr<ActivationFunction> weightsActivationFunction = nullptr;
};

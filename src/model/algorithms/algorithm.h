#pragma once

#include "model/activation_functions/activation_function.h"
#include "model/entities/calculation_fcm.h"

#include <memory>

class PredictionAlgorithm {
public:
    PredictionAlgorithm(std::shared_ptr<ActivationFunction> activationFunction) : activationFunction(activationFunction) {}

    virtual CalculationFCM step(const CalculationFCM& fcm) const = 0;

    virtual ~PredictionAlgorithm() = default;

protected:
    std::shared_ptr<ActivationFunction> activationFunction = nullptr;
};

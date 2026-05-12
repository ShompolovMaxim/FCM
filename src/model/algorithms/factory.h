#pragma once

#include "algorithm.h"

#include "model/prediction/prediction_parameters.h"

class AlgorithmsFactory {
public:
    std::shared_ptr<PredictionAlgorithm> create(const PredictionParameters& predictionParameters, std::shared_ptr<ActivationFunction> conceptsActivationFunction,
                                                std::shared_ptr<ActivationFunction> weightsActivationFunction);
};



#pragma once

#include "algorithm.h"

#include "presenter/prediction_parameters.h"

class AlgorithmsFabric {
public:
    std::shared_ptr<PredictionAlgorithm> create(const PredictionParameters& predictionParameters, std::shared_ptr<ActivationFunction> conceptsActivationFunction,
                                                std::shared_ptr<ActivationFunction> weightsActivationFunction);
};


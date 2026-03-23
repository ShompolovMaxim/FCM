#pragma once

#include "algorithm.h"

#include <QString>

class AlgorithmsFabric {
public:
    std::shared_ptr<PredictionAlgorithm> create(QString name, std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction);
};


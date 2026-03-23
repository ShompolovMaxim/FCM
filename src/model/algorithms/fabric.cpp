#include "fabric.h"
#include "standard.h"
#include "weights_prediction.h"

std::shared_ptr<PredictionAlgorithm> AlgorithmsFabric::create(QString name, std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) {
    if (name == "const weights") {
        return std::make_shared<StandardPredictionAlgorithm>(conceptsActivationFunction, weightsActivationFunction);
    }
    if (name == "changing weights") {
        return std::make_shared<WeightsPredictionAlgorithm>(conceptsActivationFunction, weightsActivationFunction);
    }
    return nullptr;
}

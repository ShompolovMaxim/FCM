#include "fabric.h"

#include "standard.h"
#include "standard_fuzzy.h"
#include "weights_prediction.h"
#include "weights_prediction_fuzzy.h"

std::shared_ptr<PredictionAlgorithm> AlgorithmsFabric::create(const PredictionParameters& predictionParameters, std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) {
    if (predictionParameters.useFuzzyValues) {
        if (predictionParameters.algorithm == "const weights") {
            return std::make_shared<StandardFuzzyAlgorithm>(conceptsActivationFunction, weightsActivationFunction);
        }
        if (predictionParameters.algorithm == "changing weights") {
            return std::make_shared<WeightsPredictionFuzzyAlgorithm>(conceptsActivationFunction, weightsActivationFunction);
        }
    } else {
        if (predictionParameters.algorithm == "const weights") {
            return std::make_shared<StandardPredictionAlgorithm>(conceptsActivationFunction, weightsActivationFunction);
        }
        if (predictionParameters.algorithm == "changing weights") {
            return std::make_shared<WeightsPredictionAlgorithm>(conceptsActivationFunction, weightsActivationFunction);
        }
    }
    return nullptr;
}

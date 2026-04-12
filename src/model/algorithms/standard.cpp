#include "standard.h"

StandardPredictionAlgorithm::StandardPredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM StandardPredictionAlgorithm::step(const CalculationFCM& fcm) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;

    for (const auto& [_, weight] : fcm.weights) {
        result.concepts[weight.toConceptId].value += fcm.concepts.at(weight.fromConceptId).value * weight.value;
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].value = conceptsActivationFunction->activate(result.concepts[id].value);
    }

    return result;
}

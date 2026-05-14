#include "weights_prediction.h"

WeightsPredictionAlgorithm::WeightsPredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM WeightsPredictionAlgorithm::step(const CalculationFCM& fcm, size_t currentStep) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;

    for (const auto& [id, weight] : fcm.weights) {
        const auto toConceptIt = fcm.concepts.find(weight.toConceptId);
        const auto fromConceptIt = fcm.concepts.find(weight.fromConceptId);
        if (toConceptIt == fcm.concepts.end() || fromConceptIt == fcm.concepts.end()) {
            continue;
        }

        if (toConceptIt->second.startStep <= currentStep && fromConceptIt->second.startStep <= currentStep) {
            result.concepts[weight.toConceptId].value += fromConceptIt->second.value * weight.value;
            result.weights[id].value += fromConceptIt->second.value * toConceptIt->second.value;
        }
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].value = conceptsActivationFunction->activate(result.concepts[id].value);
    }
    for (const auto& [id, _] : result.weights) {
        result.weights[id].value = weightsActivationFunction->activate(result.weights[id].value);
    }

    return result;
}

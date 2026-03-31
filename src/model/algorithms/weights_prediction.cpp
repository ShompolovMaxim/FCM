#include "weights_prediction.h"

WeightsPredictionAlgorithm::WeightsPredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM WeightsPredictionAlgorithm::step(const CalculationFCM& fcm) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;

    for (const auto& [id, weight] : fcm.weights) {
        result.concepts[weight.toConceptId].value = fcm.concepts.at(weight.toConceptId).value + fcm.concepts.at(weight.fromConceptId).value * weight.value;
        result.weights[id].value = fcm.weights.at(id).value + fcm.concepts.at(weight.fromConceptId).value * fcm.concepts.at(weight.toConceptId).value;
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].value = conceptsActivationFunction->activate(result.concepts[id].value);
    }
    for (const auto& [id, _] : result.weights) {
        result.weights[id].value = conceptsActivationFunction->activate(result.weights[id].value);
    }

    return result;
}

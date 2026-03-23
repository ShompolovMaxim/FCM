#include "weights_prediction.h"

WeightsPredictionAlgorithm::WeightsPredictionAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM WeightsPredictionAlgorithm::step(const CalculationFCM& fcm) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;
    result.conceptsStartSteps = fcm.conceptsStartSteps;

    for (const auto& [id, weight] : fcm.weights) {
        result.concepts[weight.toConceptId] = fcm.concepts.at(weight.toConceptId) + fcm.concepts.at(weight.fromConceptId) * weight.value;
        result.weights[id].value = weightsActivationFunction->activate(fcm.weights.at(id).value + fcm.concepts.at(weight.fromConceptId) * fcm.concepts.at(weight.toConceptId));
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id] = conceptsActivationFunction->activate(result.concepts[id]);
    }

    return result;
}

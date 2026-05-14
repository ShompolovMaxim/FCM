#include "weights_prediction_fuzzy.h"

WeightsPredictionFuzzyAlgorithm::WeightsPredictionFuzzyAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM WeightsPredictionFuzzyAlgorithm::step(const CalculationFCM& fcm, size_t currentStep) const {
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
            result.concepts[weight.toConceptId].triangularFuzzyValue = toConceptIt->second.triangularFuzzyValue +
                                                                       fromConceptIt->second.triangularFuzzyValue * weight.triangularFuzzyValue;
            result.weights[id].triangularFuzzyValue = fcm.weights.at(id).triangularFuzzyValue +
                                                      fromConceptIt->second.triangularFuzzyValue * toConceptIt->second.triangularFuzzyValue;
        }
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].triangularFuzzyValue = conceptsActivationFunction->activate(result.concepts[id].triangularFuzzyValue);
    }
    for (const auto& [id, _] : result.weights) {
        result.weights[id].triangularFuzzyValue = weightsActivationFunction->activate(result.weights[id].triangularFuzzyValue);
    }

    return result;
}


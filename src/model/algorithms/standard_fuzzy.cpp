#include "standard_fuzzy.h"

StandardFuzzyAlgorithm::StandardFuzzyAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM StandardFuzzyAlgorithm::step(const CalculationFCM& fcm, size_t currentStep) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;

    for (const auto& [_, weight] : fcm.weights) {
        const auto toConceptIt = fcm.concepts.find(weight.toConceptId);
        const auto fromConceptIt = fcm.concepts.find(weight.fromConceptId);
        if (toConceptIt == fcm.concepts.end() || fromConceptIt == fcm.concepts.end()) {
            continue;
        }

        if (toConceptIt->second.startStep <= currentStep && fromConceptIt->second.startStep <= currentStep) {
            result.concepts[weight.toConceptId].triangularFuzzyValue = toConceptIt->second.triangularFuzzyValue +
                                                                       fromConceptIt->second.triangularFuzzyValue * weight.triangularFuzzyValue;
        }
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].triangularFuzzyValue = conceptsActivationFunction->activate(result.concepts[id].triangularFuzzyValue);
    }

    return result;
}

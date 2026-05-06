#include "standard_fuzzy.h"

StandardFuzzyAlgorithm::StandardFuzzyAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM StandardFuzzyAlgorithm::step(const CalculationFCM& fcm, size_t currentStep) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;

    for (const auto& [_, weight] : fcm.weights) {
        if (fcm.concepts.at(weight.toConceptId).startStep <= currentStep && fcm.concepts.at(weight.fromConceptId).startStep <= currentStep) {
            result.concepts[weight.toConceptId].triangularFuzzyValue = fcm.concepts.at(weight.toConceptId).triangularFuzzyValue +
                                                                       fcm.concepts.at(weight.fromConceptId).triangularFuzzyValue * weight.triangularFuzzyValue;
        }
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].triangularFuzzyValue = conceptsActivationFunction->activate(result.concepts[id].triangularFuzzyValue);
    }

    return result;
}

#include "weights_prediction_fuzzy.h"

WeightsPredictionFuzzyAlgorithm::WeightsPredictionFuzzyAlgorithm(std::shared_ptr<ActivationFunction> conceptsActivationFunction, std::shared_ptr<ActivationFunction> weightsActivationFunction) :
    PredictionAlgorithm(conceptsActivationFunction, weightsActivationFunction) {}

CalculationFCM WeightsPredictionFuzzyAlgorithm::step(const CalculationFCM& fcm) const {
    CalculationFCM result;
    result.concepts = fcm.concepts;
    result.weights = fcm.weights;

    for (const auto& [id, weight] : fcm.weights) {
        result.concepts[weight.toConceptId].triangularFuzzyValue = fcm.concepts.at(weight.toConceptId).triangularFuzzyValue +
                                                                   fcm.concepts.at(weight.fromConceptId).triangularFuzzyValue * weight.triangularFuzzyValue;
        result.weights[id].triangularFuzzyValue = fcm.weights.at(id).triangularFuzzyValue + fcm.concepts.at(weight.fromConceptId).triangularFuzzyValue *
                                                                                                fcm.concepts.at(weight.toConceptId).triangularFuzzyValue;
    }
    for (const auto& [id, _] : result.concepts) {
        result.concepts[id].triangularFuzzyValue = conceptsActivationFunction->activate(result.concepts[id].triangularFuzzyValue);
        result.weights[id].triangularFuzzyValue = conceptsActivationFunction->activate(result.weights[id].triangularFuzzyValue);
    }

    return result;
}


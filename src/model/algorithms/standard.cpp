#include "standard.h"

StandardPredictionAlgorithm::StandardPredictionAlgorithm(std::shared_ptr<ActivationFunction> activationFunction) : PredictionAlgorithm(activationFunction) {}

CalculationFCM StandardPredictionAlgorithm::step(const CalculationFCM& fcm) const {
    CalculationFCM result;
    result.concepts.resize(fcm.concepts.size());
    result.weights = fcm.weights;
    result.conceptsStartSteps = fcm.conceptsStartSteps;
    for (size_t i = 0; i < fcm.concepts.size(); ++i) {
        result.concepts[i] = fcm.concepts[i];
        for (size_t j = 0; j < fcm.concepts.size(); ++j) {
            result.concepts[i] = activationFunction->activate(result.concepts[i] + fcm.concepts[j] * fcm.weights[j][i]);
        }
    }
    return result;
}

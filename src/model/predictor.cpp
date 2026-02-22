#include "predictor.h"
#include <qdebug.h>

Predictor::Predictor(PredictionParameters predictionParameters, const CalculationFCM& fcm) : _predictionParameters(predictionParameters) {
    _fcms.push_back(fcm);
}

void Predictor::perform() {
    auto activationFunction = ActivationFunctionsFabric().create(_predictionParameters.activationFunction, ElementType::Node, 1);
    auto algorithm = StandardPredictionAlgorithm(activationFunction);
    for (size_t i = 0; i < _predictionParameters.fixedSteps; ++i) {
        auto next = algorithm.step(_fcms[i]);

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _fcms.push_back(next);
            ++_count;
        }
    }
}

CalculationFCM Predictor::getFCM(size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    return _fcms[step];
}

double Predictor::getCount() {
    return _count.load();
}

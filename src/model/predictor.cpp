#include "activation_functions/fabric.h"

#include "algorithms/fabric.h"

#include "metrics/fabric.h"
#include "stop_conditions/fabric.h"

#include "element_type.h"
#include "metrics_manager.h"
#include "predictor.h"

Predictor::Predictor(PredictionParameters predictionParameters, const CalculationFCM& fcm) : _predictionParameters(predictionParameters) {
    _fcms.push_back(fcm);
}

void Predictor::perform() {
    auto conceptActivationFunction = ActivationFunctionsFabric().create(_predictionParameters.activationFunction, ElementType::Node, 1);
    auto weightActivationFunction = ActivationFunctionsFabric().create(_predictionParameters.activationFunction, ElementType::Edge, 1);
    auto algorithm = AlgorithmsFabric().create(_predictionParameters, conceptActivationFunction, weightActivationFunction);
    auto metricsManager = MetricsManager(MetricsFabric().create(_predictionParameters.metric), _predictionParameters);
    auto stopCondition = StopConditionsFabric().create(_predictionParameters);
    while (!stopRequested.load()) {
        if (stopCondition->finished(_fcms)) {
            break;
        }

        auto next = algorithm->step(_fcms[_fcms.size() - 1]);
        next.metricValue = metricsManager.calculate(_fcms[_fcms.size() - 1], next);

        if (stopRequested.load()) {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _fcms.push_back(next);
            ++_count;
        }
    }
    finished = true;
}

void Predictor::requestStop() {
    stopRequested = true;
}

CalculationFCM Predictor::getFCM(size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    return _fcms[step];
}

size_t Predictor::getCount() {
    return _count.load();
}

bool Predictor::getFinished() {
    return finished.load();
}

std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> Predictor::getConceptHistoryValues(QUuid conceptId, size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_predictionParameters.useFuzzyValues) {
        std::vector<TriangularFuzzyValue> result;
        result.reserve(step + 1);
        for (size_t i = 0; i <= step; ++i) {
            result.push_back(_fcms[i].concepts[conceptId].triangularFuzzyValue);
        }
        return result;
    }
    std::vector<double> result;
    result.reserve(step + 1);
    for (size_t i = 0; i <= step; ++i) {
        result.push_back(_fcms[i].concepts[conceptId].value);
    }
    return result;
}

std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> Predictor::getWeightHistoryValues(QUuid weightId, size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_predictionParameters.useFuzzyValues) {
        std::vector<TriangularFuzzyValue> result;
        result.reserve(step + 1);
        for (size_t i = 0; i <= step; ++i) {
            result.push_back(_fcms[i].weights[weightId].triangularFuzzyValue);
        }
        return result;
    }
    std::vector<double> result;
    result.reserve(step + 1);
    for (size_t i = 0; i <= step; ++i) {
        result.push_back(_fcms[i].weights[weightId].value);
    }
    return result;
}

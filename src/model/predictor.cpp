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
    auto algorithm = AlgorithmsFabric().create(_predictionParameters.algorithm, conceptActivationFunction, weightActivationFunction);
    auto metricsManager = MetricsManager(MetricsFabric().create(_predictionParameters.metric), _predictionParameters.algorithm == "changing weights");
    auto stopCondition = StopConditionsFabric().create(_predictionParameters);
    while (!stopCondition->finished(_fcms)) {
        auto next = algorithm->step(_fcms[_fcms.size() - 1]);
        next.metricValue = metricsManager.calculate(_fcms[_fcms.size() - 1], next);

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _fcms.push_back(next);
            ++_count;
        }
    }
    finished = true;
}

CalculationFCM Predictor::getFCM(size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    return _fcms[step];
}

double Predictor::getCount() {
    return _count.load();
}

double Predictor::getFinished() {
    return finished.load();
}

std::vector<double> Predictor::getConceptHistoryValues(size_t conceptId, size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<double> result;
    result.reserve(step + 1);
    for (size_t i = 0; i <= step; ++i) {
        result.push_back(_fcms[i].concepts[conceptId]);
    }
    return result;
}

std::vector<double> Predictor::getWeightHistoryValues(size_t weightId, size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<double> result;
    result.reserve(step + 1);
    for (size_t i = 0; i <= step; ++i) {
        result.push_back(_fcms[i].weights[weightId].value);
    }
    return result;
}

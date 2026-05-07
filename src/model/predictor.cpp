#include "activation_functions/fabric.h"

#include "algorithms/fabric.h"

#include "common/crash_log.h"
#include "metrics/fabric.h"
#include "stop_conditions/fabric.h"

#include "element_type.h"
#include "metrics_manager.h"
#include "predictor.h"

Predictor::Predictor(PredictionParameters predictionParameters, const CalculationFCM& fcm) : _predictionParameters(predictionParameters) {
    _fcms.push_back(fcm);
}

void Predictor::perform() {
    auto conceptActivationFunction = ActivationFunctionsFabric().create(_predictionParameters.activationFunction, ElementType::Node, _predictionParameters.fuzzinessDegree);
    auto weightActivationFunction = ActivationFunctionsFabric().create(_predictionParameters.activationFunction, ElementType::Edge, _predictionParameters.fuzzinessDegree);
    auto algorithm = AlgorithmsFabric().create(_predictionParameters, conceptActivationFunction, weightActivationFunction);
    auto metricsManager = MetricsManager(MetricsFabric().create(_predictionParameters.metric), _predictionParameters);
    auto stopCondition = StopConditionsFabric().create(_predictionParameters);
    if (!conceptActivationFunction || !weightActivationFunction || !algorithm || !stopCondition) {
        Logger::warn("Predictor dependency missing");
        finished = true;
        return;
    }
    while (!stopRequested.load()) {
        if (stopCondition->finished(_fcms)) {
            break;
        }

        auto next = algorithm->step(_fcms[_fcms.size() - 1], _fcms.size() - 1);
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
    if (step >= _fcms.size()) {
        Logger::warn("Predictor step invalid");
        return _fcms.empty() ? CalculationFCM{} : _fcms.back();
    }
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
    if (_fcms.empty() || step >= _fcms.size()) {
        Logger::warn("Predictor step invalid");
        if (_predictionParameters.useFuzzyValues) {
            return std::vector<TriangularFuzzyValue>{};
        }
        return std::vector<double>{};
    }
    if (_predictionParameters.useFuzzyValues) {
        std::vector<TriangularFuzzyValue> result;
        result.reserve(step + 1);
        for (size_t i = 0; i <= step; ++i) {
            const auto conceptIt = _fcms[i].concepts.find(conceptId);
            if (conceptIt == _fcms[i].concepts.end()) {
                Logger::warn("Predictor concept missing");
                return result;
            }
            result.push_back(conceptIt->second.triangularFuzzyValue);
        }
        return result;
    }
    std::vector<double> result;
    result.reserve(step + 1);
    for (size_t i = 0; i <= step; ++i) {
        const auto conceptIt = _fcms[i].concepts.find(conceptId);
        if (conceptIt == _fcms[i].concepts.end()) {
            Logger::warn("Predictor concept missing");
            return result;
        }
        result.push_back(conceptIt->second.value);
    }
    return result;
}

std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> Predictor::getWeightHistoryValues(QUuid weightId, size_t step) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_fcms.empty() || step >= _fcms.size()) {
        Logger::warn("Predictor step invalid");
        if (_predictionParameters.useFuzzyValues) {
            return std::vector<TriangularFuzzyValue>{};
        }
        return std::vector<double>{};
    }
    if (_predictionParameters.useFuzzyValues) {
        std::vector<TriangularFuzzyValue> result;
        result.reserve(step + 1);
        for (size_t i = 0; i <= step; ++i) {
            const auto weightIt = _fcms[i].weights.find(weightId);
            if (weightIt == _fcms[i].weights.end()) {
                Logger::warn("Predictor weight missing");
                return result;
            }
            result.push_back(weightIt->second.triangularFuzzyValue);
        }
        return result;
    }
    std::vector<double> result;
    result.reserve(step + 1);
    for (size_t i = 0; i <= step; ++i) {
        const auto weightIt = _fcms[i].weights.find(weightId);
        if (weightIt == _fcms[i].weights.end()) {
            Logger::warn("Predictor weight missing");
            return result;
        }
        result.push_back(weightIt->second.value);
    }
    return result;
}


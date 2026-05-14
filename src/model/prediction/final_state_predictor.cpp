#include "final_state_predictor.h"

#include "common/logger.h"
#include "model/activation_functions/factory.h"

#include "model/algorithms/factory.h"

#include "model/metrics/factory.h"
#include "model/stop_conditions/factory.h"

#include "model/entities/element_type.h"

FinalStatePredictor::FinalStatePredictor(const PredictionParameters& predictionParameters) : predictionParameters(predictionParameters) {
    auto conceptActivationFunction = ActivationFunctionsFactory().create(predictionParameters.activationFunction, ElementType::Node, predictionParameters.fuzzinessDegree);
    auto weightActivationFunction = ActivationFunctionsFactory().create(predictionParameters.activationFunction, ElementType::Edge, predictionParameters.fuzzinessDegree);
    if (conceptActivationFunction && weightActivationFunction) {
        algorithm = AlgorithmsFactory().create(predictionParameters, conceptActivationFunction, weightActivationFunction);
    }
    metricsManager = std::make_shared<MetricsManager>(MetricsFactory().create(predictionParameters.metric), predictionParameters);
    stopCondition = StopConditionsFactory().create(predictionParameters);
}

CalculationFCM FinalStatePredictor::predict(const CalculationFCM& fcm) {
    fcms.clear();
    fcms.push_back(fcm);
    if (!algorithm || !stopCondition) {
        Logger::warn("Final state predictor dependency missing");
        return fcm;
    }
    while (!stopRequested.load()) {
        if (stopCondition->finished(fcms)) {
            break;
        }

        auto next = algorithm->step(fcms[fcms.size() - 1], fcms.size() - 1);
        next.metricValue = metricsManager->calculate(fcms[fcms.size() - 1], next);

        if (stopRequested.load()) {
            break;
        }

        fcms.push_back(next);
    }
    return fcms[fcms.size() - 1];
}

void FinalStatePredictor::requestStop() {
    stopRequested = true;
}


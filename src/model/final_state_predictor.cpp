#include "final_state_predictor.h"

#include "activation_functions/fabric.h"

#include "algorithms/fabric.h"

#include "metrics/fabric.h"
#include "stop_conditions/fabric.h"

#include "element_type.h"

FinalStatePredictor::FinalStatePredictor(const PredictionParameters& predictionParameters) : predictionParameters(predictionParameters) {
    auto conceptActivationFunction = ActivationFunctionsFabric().create(predictionParameters.activationFunction, ElementType::Node, 1);
    auto weightActivationFunction = ActivationFunctionsFabric().create(predictionParameters.activationFunction, ElementType::Edge, 1);
    algorithm = AlgorithmsFabric().create(predictionParameters, conceptActivationFunction, weightActivationFunction);
    metricsManager = std::make_shared<MetricsManager>(MetricsFabric().create(predictionParameters.metric), predictionParameters);
    stopCondition = StopConditionsFabric().create(predictionParameters);
}

CalculationFCM FinalStatePredictor::predict(const CalculationFCM& fcm) {
    fcms.clear();
    fcms.push_back(fcm);
    while (!stopCondition->finished(fcms)) {
        auto next = algorithm->step(fcms[fcms.size() - 1]);
        next.metricValue = metricsManager->calculate(fcms[fcms.size() - 1], next);
        fcms.push_back(next);
    }
    return fcms[fcms.size() - 1];
}

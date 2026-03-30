#include "metrics_manager.h"

MetricsManager::MetricsManager(std::shared_ptr<Metric> metric, const PredictionParameters& predictionParameters)
    : metric(metric), predictionParameters(predictionParameters) {}

double MetricsManager::calculate(const CalculationFCM& oldFcm, const CalculationFCM& newFcm) {
    std::vector<double> oldValues;
    std::vector<double> newValues;
    int multiplyer = predictionParameters.useFuzzyValues ? 3 : 1;
    if (predictionParameters.algorithm == "changing weights") {
        oldValues.reserve((oldFcm.concepts.size() + oldFcm.weights.size()) * multiplyer);
        newValues.reserve((oldFcm.concepts.size() + oldFcm.weights.size()) * multiplyer);
    } else {
        oldValues.reserve(oldFcm.concepts.size() * multiplyer);
        newValues.reserve(oldFcm.concepts.size() * multiplyer);
    }
    for (const auto [id, _] : oldFcm.concepts) {
        if (predictionParameters.useFuzzyValues) {
            oldValues.push_back(oldFcm.concepts.at(id).triangularFuzzyValue.l);
            oldValues.push_back(oldFcm.concepts.at(id).triangularFuzzyValue.m);
            oldValues.push_back(oldFcm.concepts.at(id).triangularFuzzyValue.u);
            newValues.push_back(newFcm.concepts.at(id).triangularFuzzyValue.l);
            newValues.push_back(newFcm.concepts.at(id).triangularFuzzyValue.m);
            newValues.push_back(newFcm.concepts.at(id).triangularFuzzyValue.u);
        } else {
            oldValues.push_back(oldFcm.concepts.at(id).value);
            newValues.push_back(newFcm.concepts.at(id).value);
        }
    }
    if (predictionParameters.algorithm == "changing weights") {
        for (const auto [id, _] : oldFcm.weights) {
            if (predictionParameters.useFuzzyValues) {
                oldValues.push_back(oldFcm.weights.at(id).triangularFuzzyValue.l);
                oldValues.push_back(oldFcm.weights.at(id).triangularFuzzyValue.m);
                oldValues.push_back(oldFcm.weights.at(id).triangularFuzzyValue.u);
                newValues.push_back(newFcm.weights.at(id).triangularFuzzyValue.l);
                newValues.push_back(newFcm.weights.at(id).triangularFuzzyValue.m);
                newValues.push_back(newFcm.weights.at(id).triangularFuzzyValue.u);
            } else {
                oldValues.push_back(oldFcm.weights.at(id).value);
                newValues.push_back(newFcm.weights.at(id).value);
            }
        }
    }
    return metric->calculate(oldValues, newValues);
}

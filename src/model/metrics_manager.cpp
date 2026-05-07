#include "metrics_manager.h"

#include "common/crash_log.h"

MetricsManager::MetricsManager(std::shared_ptr<Metric> metric, const PredictionParameters& predictionParameters)
    : metric(metric), predictionParameters(predictionParameters) {}

double MetricsManager::calculate(const CalculationFCM& oldFcm, const CalculationFCM& newFcm) {
    if (!metric) {
        Logger::warn("Metric missing");
        return 0.0;
    }

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
        const auto newConceptIt = newFcm.concepts.find(id);
        if (newConceptIt == newFcm.concepts.end()) {
            Logger::warn("Metric concept missing");
            return 0.0;
        }
        if (predictionParameters.useFuzzyValues) {
            oldValues.push_back(oldFcm.concepts.at(id).triangularFuzzyValue.l);
            oldValues.push_back(oldFcm.concepts.at(id).triangularFuzzyValue.m);
            oldValues.push_back(oldFcm.concepts.at(id).triangularFuzzyValue.u);
            newValues.push_back(newConceptIt->second.triangularFuzzyValue.l);
            newValues.push_back(newConceptIt->second.triangularFuzzyValue.m);
            newValues.push_back(newConceptIt->second.triangularFuzzyValue.u);
        } else {
            oldValues.push_back(oldFcm.concepts.at(id).value);
            newValues.push_back(newConceptIt->second.value);
        }
    }
    if (predictionParameters.algorithm == "changing weights") {
        for (const auto [id, _] : oldFcm.weights) {
            const auto newWeightIt = newFcm.weights.find(id);
            if (newWeightIt == newFcm.weights.end()) {
                Logger::warn("Metric weight missing");
                return 0.0;
            }
            if (predictionParameters.useFuzzyValues) {
                oldValues.push_back(oldFcm.weights.at(id).triangularFuzzyValue.l);
                oldValues.push_back(oldFcm.weights.at(id).triangularFuzzyValue.m);
                oldValues.push_back(oldFcm.weights.at(id).triangularFuzzyValue.u);
                newValues.push_back(newWeightIt->second.triangularFuzzyValue.l);
                newValues.push_back(newWeightIt->second.triangularFuzzyValue.m);
                newValues.push_back(newWeightIt->second.triangularFuzzyValue.u);
            } else {
                oldValues.push_back(oldFcm.weights.at(id).value);
                newValues.push_back(newWeightIt->second.value);
            }
        }
    }
    return metric->calculate(oldValues, newValues);
}


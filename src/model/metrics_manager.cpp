#include "metrics_manager.h"

MetricsManager::MetricsManager(std::shared_ptr<Metric> metric, bool weightsChange) : metric(metric), weightsChange(weightsChange) {}

double MetricsManager::calculate(const CalculationFCM& oldFcm, const CalculationFCM& newFcm) {
    std::vector<double> oldValues;
    std::vector<double> newValues;
    if (weightsChange) {
        oldValues.reserve(oldFcm.concepts.size() + oldFcm.weights.size());
        newValues.reserve(oldFcm.concepts.size() + oldFcm.weights.size());
    } else {
        oldValues.reserve(oldFcm.concepts.size());
        newValues.reserve(oldFcm.concepts.size());
    }
    for (const auto [id, _] : oldFcm.concepts) {
        oldValues.push_back(oldFcm.concepts.at(id));
        newValues.push_back(newFcm.concepts.at(id));
    }
    if (weightsChange) {
        for (const auto [id, _] : oldFcm.weights) {
            oldValues.push_back(oldFcm.weights.at(id).value);
            newValues.push_back(newFcm.weights.at(id).value);
        }
    }
    return metric->calculate(oldValues, newValues);
}

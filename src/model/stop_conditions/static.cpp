#include "static.h"

StaticCondition::StaticCondition(const PredictionParameters& predictionParameters) : StopCondition(predictionParameters) {}

bool StaticCondition::finished(const std::vector<CalculationFCM>& fcms) {
    if (fcms.size() < predictionParameters.stepsLessThreshold + 1) {
        return false;
    }
    for (size_t i = fcms.size() - predictionParameters.stepsLessThreshold; i < fcms.size(); ++i) {
        if (fcms[i].metricValue > predictionParameters.threshold) {
            return false;
        }
    }
    return true;
}

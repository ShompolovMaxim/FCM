#include "fixed_steps.h"

FixedStepsCondition::FixedStepsCondition(const PredictionParameters& predictionParameters) : StopCondition(predictionParameters) {}

bool FixedStepsCondition::finished(const std::vector<CalculationFCM>& fcms) {
    return predictionParameters.fixedSteps + 1 == fcms.size();
}

#include "prediction_parameters.h"

bool PredictionParameters::operator==(const PredictionParameters& other) const {
    return algorithm == other.algorithm
        && useFuzzyValues == other.useFuzzyValues
        && activationFunction == other.activationFunction
        && metric == other.metric
        && predictToStatic == other.predictToStatic
        && threshold == other.threshold
        && stepsLessThreshold == other.stepsLessThreshold
        && fixedSteps == other.fixedSteps;
}

bool PredictionParameters::operator!=(const PredictionParameters& other) const {
    return !(*this == other);
}

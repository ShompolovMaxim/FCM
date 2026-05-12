#pragma once

#include "range.h"

#include "model/sensitivity_analysis/parameters.h"

#include "model/prediction/prediction_parameters.h"

template <typename T>
class ChangeIterationFactory {
public:
    static ChangeRange<T> create(const T& value, const SensitivityAnalysisParameters& parameters, const PredictionParameters& predictionParameters);
};

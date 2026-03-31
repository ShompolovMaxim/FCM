#pragma once

#include <cstdint>

struct SensitivityAnalysisParameters {
    double maxChange;
    bool changeConcepts;
    bool changeWeights;
    size_t steps = 10;
    size_t randomIterations = 100;
};

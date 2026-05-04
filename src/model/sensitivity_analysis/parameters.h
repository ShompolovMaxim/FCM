#pragma once

#include <cstdint>
#include <QString>

struct SensitivityAnalysisParameters {
    double maxChange;
    bool changeConcepts;
    bool changeWeights;
    size_t steps = 10;
    size_t randomIterations = 100;
    QString metric = "MSE";
};

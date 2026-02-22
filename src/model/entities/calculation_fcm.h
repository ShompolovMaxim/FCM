#pragma once
#include <vector>

struct CalculationFCM {
    std::vector<double> concepts;
    std::vector<std::vector<double>> weights;
    std::vector<size_t> conceptsStartSteps;
};

#pragma once

#include "stop_condition.h"

#include <memory>
#include <QString>

class StopConditionsFabric {
public:
    std::shared_ptr<StopCondition> create(const PredictionParameters& predictionParameters);
};

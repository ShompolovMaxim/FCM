#include "fabric.h"

#include "fixed_steps.h"
#include "static.h"

std::shared_ptr<StopCondition> StopConditionsFabric::create(const PredictionParameters& predictionParameters) {
    if (!predictionParameters.predictToStatic) {
        return std::make_shared<FixedStepsCondition>(predictionParameters);
    }
    return std::make_shared<StaticCondition>(predictionParameters);
}

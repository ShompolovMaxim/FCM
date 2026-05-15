#include <gtest/gtest.h>

#include "model/stop_conditions/factory.h"
#include "model/stop_conditions/fixed_steps.h"
#include "model/stop_conditions/static.h"
#include "model/prediction/prediction_parameters.h"

namespace {

std::vector<CalculationFCM> makeHistory(std::initializer_list<double> metrics) {
    std::vector<CalculationFCM> history;
    for (double metric : metrics) {
        CalculationFCM fcm;
        fcm.metricValue = metric;
        history.push_back(fcm);
    }
    return history;
}

}

TEST(StopConditionsTest, FixedStepsStopsAtLimit) {
    FixedStepsCondition condition(PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 2, 1.0});

    EXPECT_FALSE(condition.finished(makeHistory({0.0, 0.0})));
    EXPECT_TRUE(condition.finished(makeHistory({0.0, 0.0, 0.0})));
}

TEST(StopConditionsTest, StaticChecksRecentMetrics) {
    StaticCondition condition(PredictionParameters{"const weights", false, "threshold-linear", "MSE", true, 0.1, 2, 5, 1.0});

    EXPECT_FALSE(condition.finished(makeHistory({0.2, 0.05})));
    EXPECT_FALSE(condition.finished(makeHistory({0.2, 0.05, 0.2})));
    EXPECT_TRUE(condition.finished(makeHistory({0.2, 0.05, 0.1, 0.02})));
}

TEST(StopConditionsTest, FactoryCreatesExpectedType) {
    StopConditionsFactory factory;

    const auto fixed = factory.create(PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0});
    const auto stat = factory.create(PredictionParameters{"const weights", false, "threshold-linear", "MSE", true, 0.0, 1, 1, 1.0});

    EXPECT_NE(dynamic_cast<FixedStepsCondition*>(fixed.get()), nullptr);
    EXPECT_NE(dynamic_cast<StaticCondition*>(stat.get()), nullptr);
}


#include <gtest/gtest.h>

#include "model/metrics/metrics_manager.h"
#include "model/prediction/prediction_parameters.h"

#include <iterator>

namespace {

class RecordingMetric : public Metric {
public:
    double calculate(const std::vector<double>& a, const std::vector<double>& b) override {
        oldValues = a;
        newValues = b;
        return result;
    }

    std::vector<double> oldValues;
    std::vector<double> newValues;
    double result = 7.5;
};

CalculationFCM makeNumericFcm(bool includeWeights) {
    CalculationFCM fcm;
    const QUuid first = QUuid("{11111111-1111-1111-1111-111111111111}");
    const QUuid second = QUuid("{22222222-2222-2222-2222-222222222222}");
    const QUuid weightId = QUuid("{33333333-3333-3333-3333-333333333333}");

    fcm.concepts[first] = CalculationConcept{first, 0.2, {}, 0};
    fcm.concepts[second] = CalculationConcept{second, 0.4, {}, 0};
    if (includeWeights) {
        fcm.weights[weightId] = CalculationWeight{weightId, -0.5, {}, first, second};
    }
    return fcm;
}

}

TEST(MetricsManagerTest, UsesConceptValues) {
    auto metric = std::make_shared<RecordingMetric>();
    MetricsManager manager(metric, PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0});

    auto oldFcm = makeNumericFcm(true);
    auto newFcm = oldFcm;
    newFcm.concepts.begin()->second.value = 0.8;
    std::next(newFcm.concepts.begin())->second.value = 0.9;
    newFcm.weights.begin()->second.value = 0.7;

    EXPECT_DOUBLE_EQ(manager.calculate(oldFcm, newFcm), 7.5);
    EXPECT_EQ(metric->oldValues, std::vector<double>({0.2, 0.4}));
    EXPECT_EQ(metric->newValues, std::vector<double>({0.8, 0.9}));
}

TEST(MetricsManagerTest, ExpandsFuzzyValues) {
    auto metric = std::make_shared<RecordingMetric>();
    MetricsManager manager(metric, PredictionParameters{"changing weights", true, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0});

    CalculationFCM oldFcm;
    const QUuid first = QUuid("{11111111-1111-1111-1111-111111111111}");
    const QUuid second = QUuid("{22222222-2222-2222-2222-222222222222}");
    const QUuid weightId = QUuid("{33333333-3333-3333-3333-333333333333}");
    oldFcm.concepts[first] = CalculationConcept{first, 0.0, {0.1, 0.2, 0.3}, 0};
    oldFcm.concepts[second] = CalculationConcept{second, 0.0, {0.4, 0.5, 0.6}, 0};
    oldFcm.weights[weightId] = CalculationWeight{weightId, 0.0, {-0.3, -0.2, -0.1}, first, second};
    auto newFcm = oldFcm;
    newFcm.concepts.begin()->second.triangularFuzzyValue = {0.0, 0.2, 0.4};
    std::next(newFcm.concepts.begin())->second.triangularFuzzyValue = {0.5, 0.6, 0.7};
    newFcm.weights.begin()->second.triangularFuzzyValue = {-0.2, 0.0, 0.2};

    manager.calculate(oldFcm, newFcm);

    EXPECT_EQ(metric->oldValues, std::vector<double>({0.1, 0.2, 0.3, 0.4, 0.5, 0.6, -0.3, -0.2, -0.1}));
    EXPECT_EQ(metric->newValues, std::vector<double>({0.0, 0.2, 0.4, 0.5, 0.6, 0.7, -0.2, 0.0, 0.2}));
}

TEST(MetricsManagerTest, MissingConceptReturnsZero) {
    auto metric = std::make_shared<RecordingMetric>();
    MetricsManager manager(metric, PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0});

    auto oldFcm = makeNumericFcm(false);
    auto newFcm = oldFcm;
    newFcm.concepts.erase(newFcm.concepts.begin());

    EXPECT_DOUBLE_EQ(manager.calculate(oldFcm, newFcm), 0.0);
}

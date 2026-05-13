#include <gtest/gtest.h>

#include "model/prediction/final_state_predictor.h"
#include "model/prediction/predictor.h"
#include "model/prediction/prediction_parameters.h"

namespace {

struct NumericFcmFixture {
    QUuid sourceId{ "{11111111-1111-1111-1111-111111111111}" };
    QUuid targetId{ "{22222222-2222-2222-2222-222222222222}" };
    QUuid weightId{ "{33333333-3333-3333-3333-333333333333}" };
    CalculationFCM fcm;

    NumericFcmFixture() {
        fcm.concepts[sourceId] = CalculationConcept{sourceId, 1.0, {}, 0};
        fcm.concepts[targetId] = CalculationConcept{targetId, 0.0, {}, 0};
        fcm.weights[weightId] = CalculationWeight{weightId, 1.0, {}, sourceId, targetId};
    }
};

}

TEST(PredictorTest, PerformsPredictionAndTracksHistory) {
    NumericFcmFixture fixture;
    Predictor predictor(PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 2, 1.0}, fixture.fcm);

    predictor.perform();

    EXPECT_TRUE(predictor.getFinished());
    EXPECT_EQ(predictor.getCount(), 2U);

    const auto finalFcm = predictor.getFCM(2);
    EXPECT_DOUBLE_EQ(finalFcm.concepts.at(fixture.targetId).value, 1.0);
    EXPECT_DOUBLE_EQ(finalFcm.metricValue, 0.0);

    const auto conceptHistory = std::get<std::vector<double>>(predictor.getConceptHistoryValues(fixture.targetId, 2));
    EXPECT_EQ(conceptHistory, std::vector<double>({0.0, 1.0, 1.0}));

    const auto weightHistory = std::get<std::vector<double>>(predictor.getWeightHistoryValues(fixture.weightId, 2));
    EXPECT_EQ(weightHistory, std::vector<double>({1.0, 1.0, 1.0}));
}

TEST(PredictorTest, InvalidRequestsReturnSafeFallbacks) {
    NumericFcmFixture fixture;
    Predictor predictor(PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0}, fixture.fcm);

    predictor.perform();

    const auto fallbackFcm = predictor.getFCM(99);
    EXPECT_DOUBLE_EQ(fallbackFcm.concepts.at(fixture.targetId).value, 1.0);

    const auto history = std::get<std::vector<double>>(predictor.getConceptHistoryValues(fixture.targetId, 99));
    EXPECT_TRUE(history.empty());
}

TEST(FinalStatePredictorTest, PredictReturnsFinalStateForConfiguredSteps) {
    NumericFcmFixture fixture;
    FinalStatePredictor predictor(PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 2, 1.0});

    const auto result = predictor.predict(fixture.fcm);

    EXPECT_DOUBLE_EQ(result.concepts.at(fixture.targetId).value, 1.0);
    EXPECT_DOUBLE_EQ(result.metricValue, 0.0);
}

TEST(FinalStatePredictorTest, RequestStopBeforePredictKeepsInitialState) {
    NumericFcmFixture fixture;
    FinalStatePredictor predictor(PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 5, 1.0});
    predictor.requestStop();

    const auto result = predictor.predict(fixture.fcm);

    EXPECT_DOUBLE_EQ(result.concepts.at(fixture.targetId).value, 0.0);
    EXPECT_DOUBLE_EQ(result.weights.at(fixture.weightId).value, 1.0);
}

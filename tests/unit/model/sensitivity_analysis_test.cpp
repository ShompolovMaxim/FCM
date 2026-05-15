#include <gtest/gtest.h>

#include "model/sensitivity_analysis/change_iteration/factory.h"
#include "model/sensitivity_analysis/sensitivity_analizer.h"
#include "model/prediction/prediction_parameters.h"

#include <cmath>

TEST(SensitivityAnalysisTest, BuildsSymmetricRange) {
    SensitivityAnalysisParameters parameters{0.2, true, false, 2, 1, "MSE"};
    auto range = ChangeIterationFactory<CalculationConcept>::create(
        CalculationConcept{QUuid::createUuid(), 0.5, {0.2, 0.3, 0.4}, 0},
        parameters,
        PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0}
    );

    std::vector<double> values;
    std::vector<double> changes;
    for (const auto& [concept, change] : range) {
        values.push_back(concept.value);
        changes.push_back(change);
    }

    const std::vector<double> expectedValues{0.1, 0.3, 0.5, 0.7, 0.9};
    ASSERT_EQ(values.size(), expectedValues.size());
    for (size_t i = 0; i < values.size(); ++i) {
        EXPECT_NEAR(values[i], expectedValues[i], 1e-12);
    }
    EXPECT_EQ(changes, std::vector<double>({-0.4, -0.2, 0.0, 0.2, 0.4}));
}

TEST(SensitivityAnalysisTest, ProducesZeroSensitivity) {
    const QUuid conceptId("{11111111-1111-1111-1111-111111111111}");
    const QUuid weightId("{22222222-2222-2222-2222-222222222222}");
    CalculationFCM fcm;
    fcm.concepts[conceptId] = CalculationConcept{conceptId, 0.4, {0.4, 0.4, 0.4}, 0};
    fcm.weights[weightId] = CalculationWeight{weightId, 0.5, {0.5, 0.5, 0.5}, conceptId, conceptId};

    const SensitivityAnalysisParameters parameters{0.0, true, true, 1, 2, "MSE"};
    const auto predictionParameters = PredictionParameters{"const weights", false, "threshold-linear", "MSE", false, 0.0, 1, 1, 1.0};
    SensitivityAnalizer analyzer(parameters, predictionParameters);

    analyzer.analize(fcm);

    EXPECT_TRUE(analyzer.finished());
    EXPECT_TRUE(analyzer.getConceptFinished(conceptId));
    EXPECT_TRUE(analyzer.getWeightFinished(weightId));
    EXPECT_TRUE(analyzer.getFcmFinished());
    EXPECT_DOUBLE_EQ(analyzer.getConceptSensitivity(conceptId), 0.0);
    EXPECT_DOUBLE_EQ(analyzer.getWeightSensitivity(weightId), 0.0);
    EXPECT_EQ(analyzer.getConceptChangeSensitivity(conceptId).size(), 1U);
    EXPECT_EQ(analyzer.getWeightChangeSensitivity(weightId).size(), 1U);
    ASSERT_EQ(analyzer.getFcmSensitivity().size(), 1U);
    EXPECT_DOUBLE_EQ(analyzer.getFcmSensitivity().begin()->second, 0.0);
    EXPECT_DOUBLE_EQ(analyzer.getProgress(), 1.0);
}


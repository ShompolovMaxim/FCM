#include "gtest/gtest.h"
#include "model/algorithms/weights_prediction_fuzzy.h"

#include "test_utils.h"

TEST(WeightsPredictionFuzzyAlgorithmTest, UpdatesFuzzyConceptsAndWeightsWithOwnActivationFunctions) {
    auto conceptsActivationFunction = std::make_shared<ShiftActivationFunction>(1.0);
    auto weightsActivationFunction = std::make_shared<ShiftActivationFunction>(100.0);
    WeightsPredictionFuzzyAlgorithm algorithm(conceptsActivationFunction, weightsActivationFunction);
    CalculationFCM fcm = createFuzzyCalculationFCM({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {2.0, 3.0, 4.0});

    CalculationFCM result = algorithm.step(fcm, 0);
    const CalculationWeight& weight = result.weights.begin()->second;
    const TriangularFuzzyValue& fromConceptValue = result.concepts.at(weight.fromConceptId).triangularFuzzyValue;
    const TriangularFuzzyValue& toConceptValue = result.concepts.at(weight.toConceptId).triangularFuzzyValue;

    ASSERT_EQ(result.concepts.size(), 2u);
    ASSERT_EQ(result.weights.size(), 1u);
    EXPECT_DOUBLE_EQ(fromConceptValue.l, 2.0);
    EXPECT_DOUBLE_EQ(fromConceptValue.m, 3.0);
    EXPECT_DOUBLE_EQ(fromConceptValue.u, 4.0);
    EXPECT_DOUBLE_EQ(toConceptValue.l, 7.0);
    EXPECT_DOUBLE_EQ(toConceptValue.m, 12.0);
    EXPECT_DOUBLE_EQ(toConceptValue.u, 19.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.l, 106.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.m, 113.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.u, 122.0);
}

TEST(WeightsPredictionFuzzyAlgorithmTest, SkipsFuzzyConceptAndWeightUpdateBeforeStartStep) {
    auto conceptsActivationFunction = std::make_shared<ShiftActivationFunction>(1.0);
    auto weightsActivationFunction = std::make_shared<ShiftActivationFunction>(100.0);
    WeightsPredictionFuzzyAlgorithm algorithm(conceptsActivationFunction, weightsActivationFunction);
    CalculationFCM fcm = createFuzzyCalculationFCM({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {2.0, 3.0, 4.0}, 0, 2);

    CalculationFCM result = algorithm.step(fcm, 0);
    const CalculationWeight& weight = result.weights.begin()->second;
    const TriangularFuzzyValue& fromConceptValue = result.concepts.at(weight.fromConceptId).triangularFuzzyValue;
    const TriangularFuzzyValue& toConceptValue = result.concepts.at(weight.toConceptId).triangularFuzzyValue;

    EXPECT_DOUBLE_EQ(fromConceptValue.l, 2.0);
    EXPECT_DOUBLE_EQ(fromConceptValue.m, 3.0);
    EXPECT_DOUBLE_EQ(fromConceptValue.u, 4.0);
    EXPECT_DOUBLE_EQ(toConceptValue.l, 5.0);
    EXPECT_DOUBLE_EQ(toConceptValue.m, 6.0);
    EXPECT_DOUBLE_EQ(toConceptValue.u, 7.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.l, 102.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.m, 103.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.u, 104.0);
}

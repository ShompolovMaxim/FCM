#include "gtest/gtest.h"
#include "model/algorithms/standard.h"

#include "test_utils.h"

TEST(StandardPredictionAlgorithmTest, UpdatesConceptValuesAndAppliesConceptActivation) {
    auto conceptsActivationFunction = std::make_shared<ShiftActivationFunction>(1.0);
    auto weightsActivationFunction = std::make_shared<ShiftActivationFunction>(100.0);
    StandardPredictionAlgorithm algorithm(conceptsActivationFunction, weightsActivationFunction);
    CalculationFCM fcm = createNumericCalculationFCM(2.0, 1.0, 3.0);

    CalculationFCM result = algorithm.step(fcm);
    const CalculationWeight& weight = result.weights.begin()->second;

    ASSERT_EQ(result.concepts.size(), 2u);
    ASSERT_EQ(result.weights.size(), 1u);
    EXPECT_DOUBLE_EQ(result.concepts.at(weight.fromConceptId).value, 3.0);
    EXPECT_DOUBLE_EQ(result.concepts.at(weight.toConceptId).value, 8.0);
    EXPECT_DOUBLE_EQ(weight.value, 3.0);
}

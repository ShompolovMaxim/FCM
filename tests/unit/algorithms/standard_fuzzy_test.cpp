#include "gtest/gtest.h"
#include "model/algorithms/standard_fuzzy.h"

#include "test_utils.h"

TEST(StandardFuzzyAlgorithmTest, UpdatesFuzzyConceptValuesAndAppliesConceptActivation) {
    auto conceptsActivationFunction = std::make_shared<ShiftActivationFunction>(1.0);
    auto weightsActivationFunction = std::make_shared<ShiftActivationFunction>(100.0);
    StandardFuzzyAlgorithm algorithm(conceptsActivationFunction, weightsActivationFunction);
    CalculationFCM fcm = createFuzzyCalculationFCM({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {2.0, 3.0, 4.0});

    CalculationFCM result = algorithm.step(fcm);
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
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.l, 2.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.m, 3.0);
    EXPECT_DOUBLE_EQ(weight.triangularFuzzyValue.u, 4.0);
}

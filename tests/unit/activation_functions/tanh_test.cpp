#include "gtest/gtest.h"
#include "model/activation_functions/tanh.h"

#include <cmath>

TEST(TanhTest, ZeroValueReturnsZero) {
    Tanh tanh;
    EXPECT_DOUBLE_EQ(tanh.activate(0.0), 0.0);
}

TEST(TanhTest, PositiveValueMatchesFormula) {
    Tanh tanh(2.0);
    EXPECT_NEAR(tanh.activate(1.5), std::tanh(3.0), 1e-12);
}

TEST(TanhTest, NegativeValueMatchesFormula) {
    Tanh tanh(0.5);
    EXPECT_NEAR(tanh.activate(-2.0), std::tanh(-1.0), 1e-12);
}

TEST(TanhTest, LargerFuzzinessDegreeMakesFunctionSteeper) {
    Tanh lessSteep(1.0);
    Tanh moreSteep(3.0);

    EXPECT_GT(moreSteep.activate(1.0), lessSteep.activate(1.0));
    EXPECT_LT(moreSteep.activate(-1.0), lessSteep.activate(-1.0));
}

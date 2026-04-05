#include "gtest/gtest.h"
#include "model/activation_functions/sigmoid.h"

#include <cmath>

TEST(SigmoidTest, ZeroValueReturnsHalf) {
    Sigmoid sigmoid;
    EXPECT_DOUBLE_EQ(sigmoid.activate(0.0), 0.5);
}

TEST(SigmoidTest, PositiveValueMatchesFormula) {
    Sigmoid sigmoid(2.0);
    EXPECT_NEAR(sigmoid.activate(1.5), 1.0 / (1.0 + std::exp(-3.0)), 1e-12);
}

TEST(SigmoidTest, NegativeValueMatchesFormula) {
    Sigmoid sigmoid(0.5);
    EXPECT_NEAR(sigmoid.activate(-2.0), 1.0 / (1.0 + std::exp(1.0)), 1e-12);
}

TEST(SigmoidTest, LargerFuzzinessDegreeMakesFunctionSteeper) {
    Sigmoid lessSteep(1.0);
    Sigmoid moreSteep(3.0);

    EXPECT_GT(moreSteep.activate(1.0), lessSteep.activate(1.0));
    EXPECT_LT(moreSteep.activate(-1.0), lessSteep.activate(-1.0));
}

#include "gtest/gtest.h"
#include "model/activation_functions/threshold_linear.h"

TEST(ThresholdLinearTest, ValueBelowMin) {
    ThresholdLinear tl(0.0, 10.0);
    EXPECT_DOUBLE_EQ(tl.activate(-5.0), 0.0);
}

TEST(ThresholdLinearTest, ValueAboveMax) {
    ThresholdLinear tl(0.0, 10.0);
    EXPECT_DOUBLE_EQ(tl.activate(15.0), 10.0);
}

TEST(ThresholdLinearTest, ValueWithinRange) {
    ThresholdLinear tl(0.0, 10.0);
    EXPECT_DOUBLE_EQ(tl.activate(5.0), 5.0);
}

TEST(ThresholdLinearTest, ValueOnMinMaxBoundary) {
    ThresholdLinear tl(0.0, 10.0);
    EXPECT_DOUBLE_EQ(tl.activate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(tl.activate(10.0), 10.0);
}


TEST(ThresholdLinearTest, NegativeRange) {
    ThresholdLinear tl(-10.0, -1.0);
    EXPECT_DOUBLE_EQ(tl.activate(-15.0), -10.0);
    EXPECT_DOUBLE_EQ(tl.activate(-5.0), -5.0);
    EXPECT_DOUBLE_EQ(tl.activate(0.0), -1.0);
}

#include "gtest/gtest.h"
#include "model/activation_functions/trivalent.h"

TEST(TrivalentTest, ValueInLowerQuarterReturnsMin) {
    Trivalent trivalent(0.0, 8.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(2.0), 0.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(1.0), 0.0);
}

TEST(TrivalentTest, ValueInUpperQuarterReturnsMax) {
    Trivalent trivalent(0.0, 8.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(6.0), 8.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(7.0), 8.0);
}

TEST(TrivalentTest, ValueInMiddleRangeReturnsMidpoint) {
    Trivalent trivalent(0.0, 8.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(4.0), 4.0);
}

TEST(TrivalentTest, NegativeRangeIsHandledCorrectly) {
    Trivalent trivalent(-1.0, 1.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(-0.75), -1.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(0.0), 0.0);
    EXPECT_DOUBLE_EQ(trivalent.activate(0.75), 1.0);
}

#include "gtest/gtest.h"
#include "model/metrics/mape.h"

TEST(MAPETest, ReturnsZeroForEqualVectors) {
    MAPE mape;
    EXPECT_DOUBLE_EQ(mape.calculate({1.0, 2.0, 3.0}, {1.0, 2.0, 3.0}), 0.0);
}

TEST(MAPETest, CalculatesMeanAbsolutePercentageError) {
    MAPE mape;
    EXPECT_DOUBLE_EQ(mape.calculate({100.0, 200.0}, {110.0, 180.0}), 10.0);
}

TEST(MAPETest, UsesEpsilonForZeroDenominator) {
    MAPE mape;
    EXPECT_DOUBLE_EQ(mape.calculate({0.0, 100.0}, {1.0, 110.0}), 5000000005.0);
}

TEST(MAPETest, UsesShortestVectorLength) {
    MAPE mape;
    EXPECT_DOUBLE_EQ(mape.calculate({100.0, 200.0, 300.0}, {110.0, 180.0}), 10.0);
}

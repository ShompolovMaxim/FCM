#include "gtest/gtest.h"
#include "model/metrics/mse.h"

TEST(MSETest, ReturnsZeroForEqualVectors) {
    MSE mse;
    EXPECT_DOUBLE_EQ(mse.calculate({1.0, 2.0, 3.0}, {1.0, 2.0, 3.0}), 0.0);
}

TEST(MSETest, CalculatesMeanSquaredError) {
    MSE mse;
    EXPECT_DOUBLE_EQ(mse.calculate({1.0, 2.0, 3.0}, {2.0, 4.0, 4.0}), 2.0);
}

TEST(MSETest, UsesShortestVectorLength) {
    MSE mse;
    EXPECT_DOUBLE_EQ(mse.calculate({1.0, 2.0, 100.0}, {2.0, 4.0}), 2.5);
}

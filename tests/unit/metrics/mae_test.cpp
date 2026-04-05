#include "gtest/gtest.h"
#include "model/metrics/mae.h"

TEST(MAETest, ReturnsZeroForEqualVectors) {
    MAE mae;
    EXPECT_DOUBLE_EQ(mae.calculate({1.0, 2.0, 3.0}, {1.0, 2.0, 3.0}), 0.0);
}

TEST(MAETest, CalculatesMeanAbsoluteError) {
    MAE mae;
    EXPECT_DOUBLE_EQ(mae.calculate({1.0, 2.0, 3.0}, {2.0, 4.0, 4.0}), 4.0 / 3.0);
}

TEST(MAETest, UsesShortestVectorLength) {
    MAE mae;
    EXPECT_DOUBLE_EQ(mae.calculate({1.0, 2.0, 100.0}, {2.0, 4.0}), 1.5);
}

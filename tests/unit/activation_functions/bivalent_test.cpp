#include "gtest/gtest.h"
#include "model/activation_functions/bivalent.h"

TEST(BivalentTest, ValueBelowMidpointReturnsMin) {
    Bivalent bivalent(0.0, 10.0);
    EXPECT_DOUBLE_EQ(bivalent.activate(4.0), 0.0);
}

TEST(BivalentTest, ValueAboveMidpointReturnsMax) {
    Bivalent bivalent(0.0, 10.0);
    EXPECT_DOUBLE_EQ(bivalent.activate(6.0), 10.0);
}

TEST(BivalentTest, ValueAtMidpointReturnsMin) {
    Bivalent bivalent(0.0, 10.0);
    EXPECT_DOUBLE_EQ(bivalent.activate(5.0), 0.0);
}

TEST(BivalentTest, NegativeRangeIsHandledCorrectly) {
    Bivalent bivalent(-1.0, 1.0);
    EXPECT_DOUBLE_EQ(bivalent.activate(-0.2), -1.0);
    EXPECT_DOUBLE_EQ(bivalent.activate(0.2), 1.0);
}

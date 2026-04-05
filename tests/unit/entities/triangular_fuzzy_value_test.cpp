#include "gtest/gtest.h"
#include "model/entities/triangular_fuzzy_value.h"

TEST(TriangularFuzzyValueTest, PlusEqualsAddsComponents) {
    TriangularFuzzyValue value {1.0, 2.0, 3.0};
    value += TriangularFuzzyValue {4.0, 5.0, 6.0};

    EXPECT_DOUBLE_EQ(value.l, 5.0);
    EXPECT_DOUBLE_EQ(value.m, 7.0);
    EXPECT_DOUBLE_EQ(value.u, 9.0);
}

TEST(TriangularFuzzyValueTest, PlusReturnsNewValueWithAddedComponents) {
    TriangularFuzzyValue left {1.0, 2.0, 3.0};
    TriangularFuzzyValue right {4.0, 5.0, 6.0};

    TriangularFuzzyValue result = left + right;

    EXPECT_DOUBLE_EQ(result.l, 5.0);
    EXPECT_DOUBLE_EQ(result.m, 7.0);
    EXPECT_DOUBLE_EQ(result.u, 9.0);
    EXPECT_DOUBLE_EQ(left.l, 1.0);
    EXPECT_DOUBLE_EQ(left.m, 2.0);
    EXPECT_DOUBLE_EQ(left.u, 3.0);
}

TEST(TriangularFuzzyValueTest, MultiplyEqualsCalculatesBoundsAndMiddleValue) {
    TriangularFuzzyValue value {-1.0, 2.0, 3.0};
    value *= TriangularFuzzyValue {4.0, 5.0, 6.0};

    EXPECT_DOUBLE_EQ(value.l, -6.0);
    EXPECT_DOUBLE_EQ(value.m, 10.0);
    EXPECT_DOUBLE_EQ(value.u, 18.0);
}

TEST(TriangularFuzzyValueTest, MultiplyReturnsNewValueWithCalculatedBounds) {
    TriangularFuzzyValue left {-1.0, 2.0, 3.0};
    TriangularFuzzyValue right {4.0, 5.0, 6.0};

    TriangularFuzzyValue result = left * right;

    EXPECT_DOUBLE_EQ(result.l, -6.0);
    EXPECT_DOUBLE_EQ(result.m, 10.0);
    EXPECT_DOUBLE_EQ(result.u, 18.0);
    EXPECT_DOUBLE_EQ(left.l, -1.0);
    EXPECT_DOUBLE_EQ(left.m, 2.0);
    EXPECT_DOUBLE_EQ(left.u, 3.0);
}

TEST(TriangularFuzzyValueTest, DefuzzifyReturnsAverageOfComponents) {
    TriangularFuzzyValue value {1.0, 4.0, 7.0};
    EXPECT_DOUBLE_EQ(value.defuzzify(), 4.0);
}

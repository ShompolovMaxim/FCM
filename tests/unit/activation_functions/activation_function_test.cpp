#include "gtest/gtest.h"
#include "model/activation_functions/activation_function.h"

class TestActivationFunction : public ActivationFunction
{
public:
    double activate(double value) const override {
        return value * 2.0;
    }
};

TEST(ActivationFunctionTest, ActivatesTriangularFuzzyValueComponentWise) {
    TestActivationFunction activationFunction;
    TriangularFuzzyValue value {-1.0, 0.5, 2.0};

    TriangularFuzzyValue result = static_cast<ActivationFunction&>(activationFunction).activate(value);

    EXPECT_DOUBLE_EQ(result.l, -2.0);
    EXPECT_DOUBLE_EQ(result.m, 1.0);
    EXPECT_DOUBLE_EQ(result.u, 4.0);
}

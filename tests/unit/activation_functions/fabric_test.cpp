#include "gtest/gtest.h"
#include "model/activation_functions/bivalent.h"
#include "model/activation_functions/fabric.h"
#include "model/activation_functions/sigmoid.h"
#include "model/activation_functions/tanh.h"
#include "model/activation_functions/threshold_linear.h"
#include "model/activation_functions/trivalent.h"

#include <cmath>

TEST(ActivationFunctionsFabricTest, CreatesSigmoidWithSpecifiedFuzzinessDegree) {
    ActivationFunctionsFabric fabric;

    std::shared_ptr<ActivationFunction> activationFunction = fabric.create("sigmoid", ElementType::Node, 2.0);

    ASSERT_NE(activationFunction, nullptr);
    EXPECT_NE(dynamic_cast<Sigmoid*>(activationFunction.get()), nullptr);
    EXPECT_NEAR(activationFunction->activate(1.0), 1.0 / (1.0 + std::exp(-2.0)), 1e-12);
}

TEST(ActivationFunctionsFabricTest, CreatesTanhWithSpecifiedFuzzinessDegree) {
    ActivationFunctionsFabric fabric;

    std::shared_ptr<ActivationFunction> activationFunction = fabric.create("hyperbolic tangent", ElementType::Node, 2.0);

    ASSERT_NE(activationFunction, nullptr);
    EXPECT_NE(dynamic_cast<Tanh*>(activationFunction.get()), nullptr);
    EXPECT_NEAR(activationFunction->activate(1.0), std::tanh(2.0), 1e-12);
}

TEST(ActivationFunctionsFabricTest, CreatesNodeSpecificFunctionsInZeroOneRange) {
    ActivationFunctionsFabric fabric;

    std::shared_ptr<ActivationFunction> bivalent = fabric.create("bivalent", ElementType::Node, 0.0);
    std::shared_ptr<ActivationFunction> thresholdLinear = fabric.create("threshold-linear", ElementType::Node, 0.0);
    std::shared_ptr<ActivationFunction> trivalent = fabric.create("trivalent", ElementType::Node, 0.0);

    ASSERT_NE(bivalent, nullptr);
    ASSERT_NE(thresholdLinear, nullptr);
    ASSERT_NE(trivalent, nullptr);
    EXPECT_NE(dynamic_cast<Bivalent*>(bivalent.get()), nullptr);
    EXPECT_NE(dynamic_cast<ThresholdLinear*>(thresholdLinear.get()), nullptr);
    EXPECT_NE(dynamic_cast<Trivalent*>(trivalent.get()), nullptr);
    EXPECT_DOUBLE_EQ(bivalent->activate(0.75), 1.0);
    EXPECT_DOUBLE_EQ(thresholdLinear->activate(2.0), 1.0);
    EXPECT_DOUBLE_EQ(trivalent->activate(0.5), 0.5);
}

TEST(ActivationFunctionsFabricTest, CreatesEdgeSpecificFunctionsInMinusOneOneRange) {
    ActivationFunctionsFabric fabric;

    std::shared_ptr<ActivationFunction> bivalent = fabric.create("bivalent", ElementType::Edge, 0.0);
    std::shared_ptr<ActivationFunction> thresholdLinear = fabric.create("threshold-linear", ElementType::Edge, 0.0);
    std::shared_ptr<ActivationFunction> trivalent = fabric.create("trivalent", ElementType::Edge, 0.0);

    ASSERT_NE(bivalent, nullptr);
    ASSERT_NE(thresholdLinear, nullptr);
    ASSERT_NE(trivalent, nullptr);
    EXPECT_DOUBLE_EQ(bivalent->activate(0.1), 1.0);
    EXPECT_DOUBLE_EQ(thresholdLinear->activate(-2.0), -1.0);
    EXPECT_DOUBLE_EQ(trivalent->activate(0.0), 0.0);
}

TEST(ActivationFunctionsFabricTest, ReturnsNullptrForUnknownFunction) {
    ActivationFunctionsFabric fabric;
    EXPECT_EQ(fabric.create("unknown", ElementType::Node, 1.0), nullptr);
}

#include "gtest/gtest.h"
#include "model/activation_functions/bivalent.h"
#include "model/activation_functions/factory.h"
#include "model/activation_functions/sigmoid.h"
#include "model/activation_functions/tanh.h"
#include "model/activation_functions/threshold_linear.h"
#include "model/activation_functions/trivalent.h"

#include <cmath>

TEST(ActivationFunctionsFactoryTest, CreatesSigmoidWithSpecifiedFuzzinessDegree) {
    ActivationFunctionsFactory factory;

    std::shared_ptr<ActivationFunction> activationFunction = factory.create("sigmoid", ElementType::Node, 2.0);

    ASSERT_NE(activationFunction, nullptr);
    EXPECT_NE(dynamic_cast<Sigmoid*>(activationFunction.get()), nullptr);
    EXPECT_NEAR(activationFunction->activate(1.0), 1.0 / (1.0 + std::exp(-2.0)), 1e-12);
}

TEST(ActivationFunctionsFactoryTest, CreatesTanhWithSpecifiedFuzzinessDegree) {
    ActivationFunctionsFactory factory;

    std::shared_ptr<ActivationFunction> activationFunction = factory.create("hyperbolic tangent", ElementType::Node, 2.0);

    ASSERT_NE(activationFunction, nullptr);
    EXPECT_NE(dynamic_cast<Tanh*>(activationFunction.get()), nullptr);
    EXPECT_NEAR(activationFunction->activate(1.0), std::tanh(2.0), 1e-12);
}

TEST(ActivationFunctionsFactoryTest, CreatesNodeSpecificFunctionsInZeroOneRange) {
    ActivationFunctionsFactory factory;

    std::shared_ptr<ActivationFunction> bivalent = factory.create("bivalent", ElementType::Node, 0.0);
    std::shared_ptr<ActivationFunction> thresholdLinear = factory.create("threshold-linear", ElementType::Node, 0.0);
    std::shared_ptr<ActivationFunction> trivalent = factory.create("trivalent", ElementType::Node, 0.0);

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

TEST(ActivationFunctionsFactoryTest, CreatesEdgeSpecificFunctionsInMinusOneOneRange) {
    ActivationFunctionsFactory factory;

    std::shared_ptr<ActivationFunction> bivalent = factory.create("bivalent", ElementType::Edge, 0.0);
    std::shared_ptr<ActivationFunction> thresholdLinear = factory.create("threshold-linear", ElementType::Edge, 0.0);
    std::shared_ptr<ActivationFunction> trivalent = factory.create("trivalent", ElementType::Edge, 0.0);

    ASSERT_NE(bivalent, nullptr);
    ASSERT_NE(thresholdLinear, nullptr);
    ASSERT_NE(trivalent, nullptr);
    EXPECT_DOUBLE_EQ(bivalent->activate(0.1), 1.0);
    EXPECT_DOUBLE_EQ(thresholdLinear->activate(-2.0), -1.0);
    EXPECT_DOUBLE_EQ(trivalent->activate(0.0), 0.0);
}

TEST(ActivationFunctionsFactoryTest, ReturnsNullptrForUnknownFunction) {
    ActivationFunctionsFactory factory;
    EXPECT_EQ(factory.create("unknown", ElementType::Node, 1.0), nullptr);
}



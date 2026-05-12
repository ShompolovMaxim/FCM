#include "gtest/gtest.h"
#include "model/algorithms/factory.h"
#include "model/algorithms/standard.h"
#include "model/algorithms/standard_fuzzy.h"
#include "model/algorithms/weights_prediction.h"
#include "model/algorithms/weights_prediction_fuzzy.h"

#include "test_utils.h"

TEST(AlgorithmsFactoryTest, CreatesStandardPredictionAlgorithm) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        createPredictionParameters("const weights", false),
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<StandardPredictionAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, CreatesWeightsPredictionAlgorithm) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        createPredictionParameters("changing weights", false),
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<WeightsPredictionAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, CreatesStandardFuzzyAlgorithm) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        createPredictionParameters("const weights", true),
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<StandardFuzzyAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, CreatesWeightsPredictionFuzzyAlgorithm) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        createPredictionParameters("changing weights", true),
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<WeightsPredictionFuzzyAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, ReturnsNullptrForUnknownAlgorithm) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        createPredictionParameters("unknown", false),
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    EXPECT_EQ(algorithm, nullptr);
}



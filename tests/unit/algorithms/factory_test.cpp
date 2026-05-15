#include "gtest/gtest.h"
#include "model/algorithms/factory.h"
#include "model/algorithms/standard.h"
#include "model/algorithms/standard_fuzzy.h"
#include "model/algorithms/weights_prediction.h"
#include "model/algorithms/weights_prediction_fuzzy.h"
#include "model/prediction/prediction_parameters.h"

#include "test_utils.h"

TEST(AlgorithmsFactoryTest, CreatesStandard) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        PredictionParameters{"const weights", false, "", "", false, 0.0, 0, 0, 1.0},
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<StandardPredictionAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, CreatesChangingWeights) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        PredictionParameters{"changing weights", false, "", "", false, 0.0, 0, 0, 1.0},
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<WeightsPredictionAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, CreatesStandardFuzzy) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        PredictionParameters{"const weights", true, "", "", false, 0.0, 0, 0, 1.0},
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<StandardFuzzyAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, CreatesChangingWeightsFuzzy) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        PredictionParameters{"changing weights", true, "", "", false, 0.0, 0, 0, 1.0},
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    ASSERT_NE(algorithm, nullptr);
    EXPECT_NE(dynamic_cast<WeightsPredictionFuzzyAlgorithm*>(algorithm.get()), nullptr);
}

TEST(AlgorithmsFactoryTest, UnknownReturnsNullptr) {
    AlgorithmsFactory factory;

    std::shared_ptr<PredictionAlgorithm> algorithm = factory.create(
        PredictionParameters{"unknown", false, "", "", false, 0.0, 0, 0, 1.0},
        std::make_shared<ShiftActivationFunction>(1.0),
        std::make_shared<ShiftActivationFunction>(2.0)
    );

    EXPECT_EQ(algorithm, nullptr);
}

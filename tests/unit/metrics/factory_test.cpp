#include "gtest/gtest.h"
#include "model/metrics/factory.h"
#include "model/metrics/mae.h"
#include "model/metrics/mape.h"
#include "model/metrics/mse.h"

TEST(MetricsFactoryTest, CreatesMSE) {
    MetricsFactory factory;

    std::shared_ptr<Metric> metric = factory.create("MSE");

    ASSERT_NE(metric, nullptr);
    EXPECT_NE(dynamic_cast<MSE*>(metric.get()), nullptr);
    EXPECT_DOUBLE_EQ(metric->calculate({1.0, 2.0}, {2.0, 4.0}), 2.5);
}

TEST(MetricsFactoryTest, CreatesMAE) {
    MetricsFactory factory;

    std::shared_ptr<Metric> metric = factory.create("MAE");

    ASSERT_NE(metric, nullptr);
    EXPECT_NE(dynamic_cast<MAE*>(metric.get()), nullptr);
    EXPECT_DOUBLE_EQ(metric->calculate({1.0, 2.0}, {2.0, 4.0}), 1.5);
}

TEST(MetricsFactoryTest, CreatesMAPE) {
    MetricsFactory factory;

    std::shared_ptr<Metric> metric = factory.create("MAPE");

    ASSERT_NE(metric, nullptr);
    EXPECT_NE(dynamic_cast<MAPE*>(metric.get()), nullptr);
    EXPECT_DOUBLE_EQ(metric->calculate({100.0, 200.0}, {110.0, 180.0}), 10.0);
}

TEST(MetricsFactoryTest, ReturnsNullptrForUnknownMetric) {
    MetricsFactory factory;
    EXPECT_EQ(factory.create("unknown"), nullptr);
}



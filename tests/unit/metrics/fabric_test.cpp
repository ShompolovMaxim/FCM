#include "gtest/gtest.h"
#include "model/metrics/fabric.h"
#include "model/metrics/mae.h"
#include "model/metrics/mape.h"
#include "model/metrics/mse.h"

TEST(MetricsFabricTest, CreatesMSE) {
    MetricsFabric fabric;

    std::shared_ptr<Metric> metric = fabric.create("MSE");

    ASSERT_NE(metric, nullptr);
    EXPECT_NE(dynamic_cast<MSE*>(metric.get()), nullptr);
    EXPECT_DOUBLE_EQ(metric->calculate({1.0, 2.0}, {2.0, 4.0}), 2.5);
}

TEST(MetricsFabricTest, CreatesMAE) {
    MetricsFabric fabric;

    std::shared_ptr<Metric> metric = fabric.create("MAE");

    ASSERT_NE(metric, nullptr);
    EXPECT_NE(dynamic_cast<MAE*>(metric.get()), nullptr);
    EXPECT_DOUBLE_EQ(metric->calculate({1.0, 2.0}, {2.0, 4.0}), 1.5);
}

TEST(MetricsFabricTest, CreatesMAPE) {
    MetricsFabric fabric;

    std::shared_ptr<Metric> metric = fabric.create("MAPE");

    ASSERT_NE(metric, nullptr);
    EXPECT_NE(dynamic_cast<MAPE*>(metric.get()), nullptr);
    EXPECT_DOUBLE_EQ(metric->calculate({100.0, 200.0}, {110.0, 180.0}), 10.0);
}

TEST(MetricsFabricTest, ReturnsNullptrForUnknownMetric) {
    MetricsFabric fabric;
    EXPECT_EQ(fabric.create("unknown"), nullptr);
}

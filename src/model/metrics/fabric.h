#pragma once

#include "metric.h"

#include <memory>
#include <QString>

class MetricsFabric {
public:
    std::shared_ptr<Metric> create(QString name);
};

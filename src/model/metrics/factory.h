#pragma once

#include "metric.h"

#include <memory>
#include <QString>

class MetricsFactory {
public:
    std::shared_ptr<Metric> create(QString name);
};


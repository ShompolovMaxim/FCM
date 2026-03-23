#include "fabric.h"

#include "mse.h"
#include "mae.h"
#include "mape.h"

std::shared_ptr<Metric> MetricsFabric::create(QString name) {
    if (name == "MSE") {
        return std::make_shared<MSE>();
    }
    if (name == "MAE") {
        return std::make_shared<MAE>();
    }
    if (name == "MAPE") {
        return std::make_shared<MAPE>();
    }
    return nullptr;
}

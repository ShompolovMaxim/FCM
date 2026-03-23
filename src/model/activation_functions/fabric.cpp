#include "fabric.h"
#include "bivalent.h"
#include "sigmoid.h"
#include "tanh.h"
#include "threshold_linear.h"
#include "trivalent.h"

std::shared_ptr<ActivationFunction> ActivationFunctionsFabric::create(QString name, ElementType elementType, double fuzzinessDegree) {
    if (name == "sigmoid") {
        return std::make_shared<Sigmoid>(fuzzinessDegree);
    }
    if (name == "hyperbolic tangent") {
        return std::make_shared<Tanh>(fuzzinessDegree);
    }
    if (elementType == ElementType::Node) {
        if (name == "bivalent") {
            return std::make_shared<Bivalent>(0.0, 1.0);
        }
        if (name == "threshold-linear") {
            return std::make_shared<ThresholdLinear>(0.0, 1.0);
        }
        if (name == "trivalent") {
            return std::make_shared<Trivalent>(0.0, 1.0);
        }
    } else {
        if (name == "bivalent") {
            return std::make_shared<Bivalent>(-1.0, 1.0);
        }
        if (name == "threshold-linear") {
            return std::make_shared<ThresholdLinear>(-1.0, 1.0);
        }
        if (name == "trivalent") {
            return std::make_shared<Trivalent>(-1.0, 1.0);
        }
    }
    return nullptr;
}

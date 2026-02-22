#include "threshold_linear.h"

ThresholdLinear::ThresholdLinear(double min, double max) : min(min), max(max) {}

double ThresholdLinear::activate(double value) const {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

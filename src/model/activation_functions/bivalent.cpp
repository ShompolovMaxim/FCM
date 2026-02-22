#include "bivalent.h"

Bivalent::Bivalent(double min, double max) : min(min), max(max) {}

double Bivalent::activate(double value) const {
    if (value > (min + max) / 2) {
        return max;
    }
    return min;
}

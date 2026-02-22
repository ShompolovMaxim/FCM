#include "trivalent.h"

Trivalent::Trivalent(double min, double max) : min(min), max(max) {}

double Trivalent::activate(double value) const {
    if (value <= min + (max - min) / 4) {
        return min;
    }
    if (value >= max - (max - min) / 4) {
        return max;
    }
    return (max + min) / 2;
}

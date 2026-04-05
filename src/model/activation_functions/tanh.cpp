#include "tanh.h"

#include <cmath>

Tanh::Tanh(double fuzzinessDegree) : fuzzinessDegree(fuzzinessDegree) {}

double Tanh::activate(double value) const {
    return std::tanh(fuzzinessDegree * value);
}

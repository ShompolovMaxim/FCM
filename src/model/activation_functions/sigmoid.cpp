#include "sigmoid.h"

#include <cmath>

Sigmoid::Sigmoid(double fuzziness_degree) : fuzzinessDegree(fuzzinessDegree) {}

double Sigmoid::activate(double value) const {
    return 1.0 / (1.0 + std::exp(-fuzzinessDegree * value));
}

#include "tanh.h"

#include <cmath>

Tanh::Tanh(double fuzziness_degree) : fuzzinessDegree(fuzzinessDegree) {}

double Tanh::activate(double value) const {
    return std::tanh(fuzzinessDegree * value);
}

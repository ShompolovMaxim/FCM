#include "mae.h"
#include <cmath>

double MAE::calculate(const std::vector<double>& a, const std::vector<double>& b) {
    double result = 0.0;
    int n = std::min(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        result += std::abs(a[i] - b[i]);
    }
    return result / n;
}

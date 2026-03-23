#include "mape.h"
#include <cmath>
#include <algorithm>

double MAPE::calculate(const std::vector<double>& a, const std::vector<double>& b) {
    double result = 0.0;
    int n = std::min(a.size(), b.size());
    const double eps = 1e-8;
    for (int i = 0; i < n; ++i) {
        double denom = std::max(std::abs(a[i]), eps);
        result += std::abs((a[i] - b[i]) / denom);
    }
    return (result / n) * 100.0;
}

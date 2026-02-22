#include "mse.h"

double MSE::calculate(const std::vector<double>& a, const std::vector<double>& b) {
    double result = 0;
    int n =  std::min(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        result += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return result / n;
}

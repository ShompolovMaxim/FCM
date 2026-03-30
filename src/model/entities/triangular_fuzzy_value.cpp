#include "triangular_fuzzy_value.h"

#include <algorithm>

TriangularFuzzyValue& TriangularFuzzyValue::operator+=(const TriangularFuzzyValue& other) {
    l += other.l;
    m += other.m;
    u += other.u;
    return *this;
}

TriangularFuzzyValue TriangularFuzzyValue::operator+(const TriangularFuzzyValue& other) const {
    TriangularFuzzyValue result = *this;
    result += other;
    return result;
}

TriangularFuzzyValue& TriangularFuzzyValue::operator*=(const TriangularFuzzyValue& other) {
    double p1 = l * other.l;
    double p2 = l * other.u;
    double p3 = u * other.l;
    double p4 = u * other.u;

    l = std::min({p1, p2, p3, p4});
    m = m * other.m;
    u = std::max({p1, p2, p3, p4});

    return *this;
}

TriangularFuzzyValue TriangularFuzzyValue::operator*(const TriangularFuzzyValue& other) const {
    TriangularFuzzyValue result = *this;
    result *= other;
    return result;
}

double TriangularFuzzyValue::defuzzify() {
    return (l + m + u) / 3;
}

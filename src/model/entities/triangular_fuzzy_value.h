#pragma once

struct TriangularFuzzyValue {
    double l = 0;
    double m = 0;
    double u = 0;

    TriangularFuzzyValue operator+(const TriangularFuzzyValue& other) const;
    TriangularFuzzyValue operator-(const TriangularFuzzyValue& other) const;
    TriangularFuzzyValue operator*(const TriangularFuzzyValue& other) const;
    TriangularFuzzyValue operator/(const double& rhs) const;
    TriangularFuzzyValue& operator+=(const TriangularFuzzyValue& other);
    TriangularFuzzyValue& operator-=(const TriangularFuzzyValue& other);
    TriangularFuzzyValue& operator*=(const TriangularFuzzyValue& other);
    TriangularFuzzyValue& operator/=(const double& rhs);

    double defuzzify() const;
};

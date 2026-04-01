#pragma once

#include "abstract.h"

#include "model/entities/triangular_fuzzy_value.h"

template <typename T>
class FuzzyChangeIterator : public ChangeIterator<T> {
public:
    FuzzyChangeIterator(T value, double step, int maxIndex, int indexL = 0, int indexM = 0, int indexU = 0);

    void next() override;

    T current() const override;
    double currentDouble() const override;

    bool equals(const ChangeIterator<T>& other) const override;

private:
    int indexL;
    int indexM;
    int indexU;
    int maxIndex;
    double step;
    T value;
    const TriangularFuzzyValue initialValue;
};

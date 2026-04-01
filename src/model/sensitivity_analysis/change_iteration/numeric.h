#pragma once
#include "abstract.h"

template <typename T>
class NumericChangeIterator : public ChangeIterator<T> {
public:
    explicit NumericChangeIterator(T value, double step, int index = 0);

    void next() override;

    T current() const override;
    double currentDouble() const override;

    bool equals(const ChangeIterator<T>& other) const override;

private:
    int index;
    double step;
    T value;
    const double initialValue;
};

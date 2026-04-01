#include "numeric.h"

#include "model/entities/calculation_concept.h"
#include "model/entities/calculation_weight.h"

template <typename T>
NumericChangeIterator<T>::NumericChangeIterator(T value, double step, int index) : value(value), initialValue(value.value), step(step), index(index) {}

template <typename T>
void NumericChangeIterator<T>::next() {
    ++index;
    value.value = initialValue + step * index;
}

template <typename T>
T NumericChangeIterator<T>::current() const {
    return value;
}

template <typename T>
double NumericChangeIterator<T>::currentDouble() const {
    return step * index;
}

template <typename T>
bool NumericChangeIterator<T>::equals(const ChangeIterator<T>& other) const {
    const auto* o = dynamic_cast<const NumericChangeIterator*>(&other);
    return o && (index == o->index) && (step == o->step);
}

template class NumericChangeIterator<CalculationWeight>;
template class NumericChangeIterator<CalculationConcept>;

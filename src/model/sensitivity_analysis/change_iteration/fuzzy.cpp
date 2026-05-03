#include "fuzzy.h"

#include "model/entities/calculation_concept.h"
#include "model/entities/calculation_weight.h"
#include <cstdlib>

template <typename T>
FuzzyChangeIterator<T>::FuzzyChangeIterator(T value, double step, int maxIndex, int indexL, int indexM, int indexU)
    : value(value), initialValue(value.triangularFuzzyValue), step(step), maxIndex(maxIndex), indexL(indexL), indexM(indexM), indexU(indexU) {
    this->value.triangularFuzzyValue = {
        initialValue.l + indexL * step,
        initialValue.m + indexM * step,
        initialValue.u + indexU * step
    };
}

template <typename T>
void FuzzyChangeIterator<T>::next() {
    if (indexL >= maxIndex) {
        indexL -= 2 * maxIndex;
        if (indexM >= maxIndex) {
            indexM -= 2 * maxIndex;
            ++indexU;
        } else {
            ++indexM;
        }
    } else {
        ++indexL;
    }

    value.triangularFuzzyValue = {
        initialValue.l + indexL * step,
        initialValue.m + indexM * step,
        initialValue.u + indexU * step
    };
}

template <typename T>
T FuzzyChangeIterator<T>::current() const {
    return value;
}

template <typename T>
double FuzzyChangeIterator<T>::currentDouble() const {
    return (std::abs(indexL) + std::abs(indexM) + std::abs(indexU)) * step / 3;
}

template <typename T>
bool FuzzyChangeIterator<T>::equals(const ChangeIterator<T>& other) const {
    const auto* o = dynamic_cast<const FuzzyChangeIterator*>(&other);
    return o && (indexL == o->indexL) && (indexM == o->indexM) && (indexU == o->indexU) && (step == o->step);
}

template class FuzzyChangeIterator<CalculationWeight>;
template class FuzzyChangeIterator<CalculationConcept>;

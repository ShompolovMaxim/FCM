#include "range.h"

#include "model/entities/calculation_concept.h"
#include "model/entities/calculation_weight.h"

template <typename T>
ChangeRange<T>::ChangeRange(std::unique_ptr<IteratorType> beginIt,
                            std::unique_ptr<IteratorType> endIt)
    : beginIter(std::move(beginIt)),
    endIter(std::move(endIt)) {}

template <typename T>
typename ChangeRange<T>::Iterator
ChangeRange<T>::begin() {
    return Iterator(beginIter.get());
}

template <typename T>
typename ChangeRange<T>::Iterator
ChangeRange<T>::end() {
    return Iterator(endIter.get());
}

template <typename T>
ChangeRange<T>::Iterator::Iterator(IteratorType* it)
    : iter(it) {}

template <typename T>
typename ChangeRange<T>::Iterator&
ChangeRange<T>::Iterator::operator++() {
    iter->next();
    return *this;
}

template <typename T>
std::pair<T, double>
ChangeRange<T>::Iterator::operator*() const {
    return { iter->current(), iter->currentDouble() };
}

template <typename T>
bool ChangeRange<T>::Iterator::operator!=(const Iterator& other) const {
    return !iter->equals(*other.iter);
}

template <typename T>
bool ChangeRange<T>::Iterator::operator==(const Iterator& other) const {
    return iter->equals(*other.iter);
}

template class ChangeRange<CalculationWeight>;
template class ChangeRange<CalculationConcept>;

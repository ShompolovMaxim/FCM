#pragma once
#include <memory>
#include <utility>

#include "abstract.h"

template <typename T>
class ChangeRange {
public:
    using IteratorType = ChangeIterator<T>;

    ChangeRange(std::unique_ptr<IteratorType> beginIt,
                std::unique_ptr<IteratorType> endIt);

    class Iterator {
    public:
        explicit Iterator(IteratorType* it);

        Iterator& operator++();
        std::pair<T, double> operator*() const;

        bool operator!=(const Iterator& other) const;
        bool operator==(const Iterator& other) const;

    private:
        IteratorType* iter;
    };

    Iterator begin();
    Iterator end();

private:
    std::unique_ptr<IteratorType> beginIter;
    std::unique_ptr<IteratorType> endIter;
};

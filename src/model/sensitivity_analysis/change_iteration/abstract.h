#pragma once

template <typename T>
class ChangeIterator {
public:
    virtual ~ChangeIterator() = default;

    virtual void next() = 0;

    virtual T current() const = 0;
    virtual double currentDouble() const = 0;

    virtual bool equals(const ChangeIterator& other) const = 0;
};

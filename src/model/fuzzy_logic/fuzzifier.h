#pragma once

#include "model/entities/fcm.h"

#include <QListWidget>
#include <map>
#include <memory>

class Fuzzifier {
public:
    Fuzzifier() {}
    virtual ~Fuzzifier() = default;

    virtual std::shared_ptr<Term> fuzzify(const std::map<size_t, std::shared_ptr<Term>>& terms, double value) = 0;
};

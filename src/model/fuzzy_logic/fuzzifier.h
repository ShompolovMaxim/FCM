#pragma once

#include "model/entities/fcm.h"

#include <QListWidget>
#include <map>
#include <memory>

class Fuzzifier {
public:
    Fuzzifier() {}
    virtual ~Fuzzifier() = default;

    virtual Term fuzzify(const std::map<size_t, Term>& terms, double value) = 0;
};

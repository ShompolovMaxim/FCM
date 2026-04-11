#pragma once

#include "model/entities/term.h"

#include <QUuid>

#include <map>
#include <memory>

class Fuzzifier {
public:
    Fuzzifier() {}
    virtual ~Fuzzifier() = default;

    virtual std::shared_ptr<Term> fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, double value) = 0;
    virtual std::shared_ptr<Term> fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, TriangularFuzzyValue value) = 0;
    virtual std::shared_ptr<Term> fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, double value, TriangularFuzzyValue fuzzyValue) = 0;
};

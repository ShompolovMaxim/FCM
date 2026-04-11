#pragma once

#include "fuzzifier.h"

class LinearFuzzifier : public Fuzzifier {
public:
    std::shared_ptr<Term> fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, double value) override;
    std::shared_ptr<Term> fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, TriangularFuzzyValue value) override;
    std::shared_ptr<Term> fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, double value, TriangularFuzzyValue fuzzyValue) override;
};


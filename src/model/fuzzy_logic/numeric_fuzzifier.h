#pragma once

#include "fuzzifier.h"

class NumericFuzzifier : public Fuzzifier {
public:
    std::shared_ptr<Term> fuzzify(const std::map<size_t, std::shared_ptr<Term>>& terms, double value) override;
};

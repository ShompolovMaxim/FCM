#pragma once

#include "fuzzifier.h"

class NumericFuzzifier : public Fuzzifier {
public:
    Term fuzzify(const std::map<size_t, Term>& terms, double value) override;
};

#include "numeric_fuzzifier.h"

#include <cmath>

std::shared_ptr<Term> NumericFuzzifier::fuzzify(const std::map<size_t, std::shared_ptr<Term>>& terms, double value) {
    auto result = (terms).begin()->second;
    for (const auto& term : terms) {
        if (std::abs(result->value - value) > std::abs(term.second->value - value)) {
            result = term.second;
        }
    }
    return result;
}

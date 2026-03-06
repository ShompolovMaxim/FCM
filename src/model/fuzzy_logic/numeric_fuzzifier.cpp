#include "numeric_fuzzifier.h"

#include <cmath>

Term NumericFuzzifier::fuzzify(const std::map<size_t, Term>& terms, double value) {
    Term result = (terms).begin()->second;
    for (const auto& term : terms) {
        if (std::abs(result.value - value) > std::abs(term.second.value - value)) {
            result = term.second;
        }
    }
    return result;
}

#include "linear_fuzzifier.h"

#include <cmath>

std::shared_ptr<Term> LinearFuzzifier::fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, double value) {
    auto result = (terms).begin()->second;
    for (const auto& term : terms) {
        if (std::abs(result->value - value) > std::abs(term.second->value - value)) {
            result = term.second;
        }
    }
    return result;
}

std::shared_ptr<Term> LinearFuzzifier::fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, TriangularFuzzyValue value) {
    auto result = (terms).begin()->second;
    for (const auto& [id, term] : terms) {
        if (std::abs(result->fuzzyValue.l - value.l) + std::abs(result->fuzzyValue.m - value.m) + std::abs(result->fuzzyValue.u - value.u) >
            std::abs(term->fuzzyValue.l - value.l) + std::abs(term->fuzzyValue.m - value.m) + std::abs(term->fuzzyValue.u - value.u)) {
            result = term;
        }
    }
    return result;
}

std::shared_ptr<Term> LinearFuzzifier::fuzzify(const std::map<QUuid, std::shared_ptr<Term>>& terms, double value, TriangularFuzzyValue fuzzyValue) {
    auto result = (terms).begin()->second;
    for (const auto& [id, term] : terms) {
        if ((std::abs(result->fuzzyValue.l - fuzzyValue.l) + std::abs(result->fuzzyValue.m - fuzzyValue.m) + std::abs(result->fuzzyValue.u - fuzzyValue.u)) / 3 +
             std::abs(result->value - value) >
            (std::abs(term->fuzzyValue.l - fuzzyValue.l) + std::abs(term->fuzzyValue.m - fuzzyValue.m) + std::abs(term->fuzzyValue.u - fuzzyValue.u)) / 3 +
            std::abs(term->value- value)) {
            result = term;
        }
    }
    return result;
}

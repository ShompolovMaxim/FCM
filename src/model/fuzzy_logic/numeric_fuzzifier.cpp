#include "numeric_fuzzifier.h"

#include <cmath>

Term NumericFuzzifier::fuzzify(std::shared_ptr<FCM> fcm, double value) {
    Term result = (fcm->terms).begin()->second;
    for (const auto& term : fcm->terms) {
        if (std::abs(result.value - value) > std::abs(term.second.value - value)) {
            result = term.second;
        }
    }
    return result;
}

#include "equality.h"

namespace {

template <typename T>
bool areEqualByValue(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) {
    if (lhs == rhs) {
        return true;
    }

    if (!lhs || !rhs) {
        return false;
    }

    return *lhs == *rhs;
}

template <typename T>
bool areEqualMapsImpl(
    const std::map<QUuid, std::shared_ptr<T>>& lhs,
    const std::map<QUuid, std::shared_ptr<T>>& rhs
) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (const auto& [id, lhsValue] : lhs) {
        const auto rhsIt = rhs.find(id);
        if (rhsIt == rhs.end() || !areEqualByValue(lhsValue, rhsIt->second)) {
            return false;
        }
    }

    return true;
}

}

bool areEqualTerms(const std::shared_ptr<Term>& lhs, const std::shared_ptr<Term>& rhs) {
    return areEqualByValue(lhs, rhs);
}

template <typename T>
bool areEqualMaps(
    const std::map<QUuid, std::shared_ptr<T>>& lhs,
    const std::map<QUuid, std::shared_ptr<T>>& rhs
) {
    return areEqualMapsImpl(lhs, rhs);
}

template bool areEqualMaps<Term>(
    const std::map<QUuid, std::shared_ptr<Term>>& lhs,
    const std::map<QUuid, std::shared_ptr<Term>>& rhs
);

template bool areEqualMaps<Concept>(
    const std::map<QUuid, std::shared_ptr<Concept>>& lhs,
    const std::map<QUuid, std::shared_ptr<Concept>>& rhs
);

template bool areEqualMaps<Weight>(
    const std::map<QUuid, std::shared_ptr<Weight>>& lhs,
    const std::map<QUuid, std::shared_ptr<Weight>>& rhs
);

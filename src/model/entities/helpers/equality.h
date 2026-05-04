#pragma once

#include "../concept.h"
#include "../term.h"
#include "../weight.h"

#include <map>
#include <memory>

bool areEqualTerms(const std::shared_ptr<Term>& lhs, const std::shared_ptr<Term>& rhs);

template <typename T>
bool areEqualMaps(
    const std::map<QUuid, std::shared_ptr<T>>& lhs,
    const std::map<QUuid, std::shared_ptr<T>>& rhs
);

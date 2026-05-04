#include "weight.h"
#include "helpers/equality.h"

bool Weight::operator==(const Weight& other) const {
    return name == other.name
        && description == other.description
        && areEqualTerms(term, other.term)
        && fromConceptId == other.fromConceptId
        && toConceptId == other.toConceptId;
}

bool Weight::operator!=(const Weight& other) const {
    return !(*this == other);
}

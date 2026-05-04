#include "concept.h"
#include "helpers/equality.h"

bool Concept::operator==(const Concept& other) const {
    return name == other.name
        && description == other.description
        && areEqualTerms(term, other.term)
        && pos == other.pos
        && startStep == other.startStep;
}

bool Concept::operator!=(const Concept& other) const {
    return !(*this == other);
}

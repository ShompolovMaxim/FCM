#include "term.h"

bool Term::operator==(const Term& other) const {
    return name == other.name
        && description == other.description
        && value == other.value
        && fuzzyValue == other.fuzzyValue
        && color == other.color
        && type == other.type;
}

bool Term::operator!=(const Term& other) const {
    return !(*this == other);
}

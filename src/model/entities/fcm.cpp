#include "fcm.h"
#include "helpers/equality.h"

bool FCM::operator==(const FCM& other) const {
    return name == other.name
        && description == other.description
        && areEqualMaps(terms, other.terms)
        && areEqualMaps(concepts, other.concepts)
        && areEqualMaps(weights, other.weights)
        && predictionParameters == other.predictionParameters
        && experiments == other.experiments;
}

bool FCM::operator!=(const FCM& other) const {
    return !(*this == other);
}

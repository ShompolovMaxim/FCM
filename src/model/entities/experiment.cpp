#include "experiment.h"
#include "helpers/equality.h"

bool Experiment::operator==(const Experiment& other) const {
    return areEqualMaps(terms, other.terms)
        && areEqualMaps(concepts, other.concepts)
        && areEqualMaps(weights, other.weights)
        && predictionParameters == other.predictionParameters
        && timestamp == other.timestamp;
}

bool Experiment::operator!=(const Experiment& other) const {
    return !(*this == other);
}

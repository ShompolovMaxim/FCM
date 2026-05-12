#include "fcm.h"
#include "helpers/equality.h"

bool FCM::checkElementsHaveValues(QString* errorMessage) const {
    for (const auto& [_, concept] : concepts) {
        if (!concept->term) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Not every concept has a value!");
            }
            return false;
        }
    }

    for (const auto& [_, weight] : weights) {
        if (!weight->term) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Not every weight has a value!");
            }
            return false;
        }
    }

    return true;
}

bool FCM::operator==(const FCM& other) const {
    return name == other.name
        && description == other.description
        && areEqualMaps(terms, other.terms)
        && areEqualMaps(concepts, other.concepts)
        && areEqualMaps(weights, other.weights)
        && predictionParameters == other.predictionParameters
        && experiments == other.experiments
        && autosaveOn == other.autosaveOn
        && autoConfigureTermsColors == other.autoConfigureTermsColors
        && autoConfigureNumericValues == other.autoConfigureNumericValues
        && autoConfigureFuzzyValues == other.autoConfigureFuzzyValues;
}

bool FCM::operator!=(const FCM& other) const {
    return !(*this == other);
}

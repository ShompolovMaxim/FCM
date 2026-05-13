#pragma once

#include "model/activation_functions/activation_function.h"
#include "model/entities/calculation/calculation_fcm.h"

#include <memory>

class ShiftActivationFunction : public ActivationFunction
{
public:
    explicit ShiftActivationFunction(double shift) : shift(shift) {}

    double activate(double value) const override {
        return value + shift;
    }

private:
    double shift;
};

inline CalculationFCM createNumericCalculationFCM(
    double fromValue,
    double toValue,
    double weightValue,
    size_t fromStartStep = 0,
    size_t toStartStep = 0
) {
    CalculationFCM fcm;

    const QUuid fromId = QUuid::createUuid();
    const QUuid toId = QUuid::createUuid();
    const QUuid weightId = QUuid::createUuid();

    fcm.concepts[fromId] = CalculationConcept {fromId, fromValue, {}, fromStartStep};
    fcm.concepts[toId] = CalculationConcept {toId, toValue, {}, toStartStep};
    fcm.weights[weightId] = CalculationWeight {weightId, weightValue, {}, fromId, toId};

    return fcm;
}

inline CalculationFCM createFuzzyCalculationFCM(
    TriangularFuzzyValue fromValue,
    TriangularFuzzyValue toValue,
    TriangularFuzzyValue weightValue,
    size_t fromStartStep = 0,
    size_t toStartStep = 0
) {
    CalculationFCM fcm;

    const QUuid fromId = QUuid::createUuid();
    const QUuid toId = QUuid::createUuid();
    const QUuid weightId = QUuid::createUuid();

    fcm.concepts[fromId] = CalculationConcept {fromId, 0.0, fromValue, fromStartStep};
    fcm.concepts[toId] = CalculationConcept {toId, 0.0, toValue, toStartStep};
    fcm.weights[weightId] = CalculationWeight {weightId, 0.0, weightValue, fromId, toId};

    return fcm;
}

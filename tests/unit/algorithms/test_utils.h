#pragma once

#include "model/activation_functions/activation_function.h"
#include "model/entities/calculation_fcm.h"
#include "presenter/prediction_parameters.h"

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

inline CalculationFCM createNumericCalculationFCM(double fromValue, double toValue, double weightValue) {
    CalculationFCM fcm;

    const QUuid fromId = QUuid::createUuid();
    const QUuid toId = QUuid::createUuid();
    const QUuid weightId = QUuid::createUuid();

    fcm.concepts[fromId] = CalculationConcept {fromId, fromValue, {}, 0};
    fcm.concepts[toId] = CalculationConcept {toId, toValue, {}, 0};
    fcm.weights[weightId] = CalculationWeight {weightId, weightValue, {}, fromId, toId};

    return fcm;
}

inline CalculationFCM createFuzzyCalculationFCM(TriangularFuzzyValue fromValue, TriangularFuzzyValue toValue, TriangularFuzzyValue weightValue) {
    CalculationFCM fcm;

    const QUuid fromId = QUuid::createUuid();
    const QUuid toId = QUuid::createUuid();
    const QUuid weightId = QUuid::createUuid();

    fcm.concepts[fromId] = CalculationConcept {fromId, 0.0, fromValue, 0};
    fcm.concepts[toId] = CalculationConcept {toId, 0.0, toValue, 0};
    fcm.weights[weightId] = CalculationWeight {weightId, 0.0, weightValue, fromId, toId};

    return fcm;
}

inline PredictionParameters createPredictionParameters(const QString& algorithm, bool useFuzzyValues) {
    return PredictionParameters {
        algorithm,
        useFuzzyValues,
        "",
        "",
        false,
        0.0,
        0,
        0
    };
}

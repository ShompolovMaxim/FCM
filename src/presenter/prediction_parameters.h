#pragma once
#include <QString>

struct PredictionParameters {
    QString algorithm;
    bool useFuzzyValues;
    QString activationFunction;
    QString metric;
    bool predictToStatic;
    double threshold;
    int stepsLessThreshold;
    int fixedSteps;
};

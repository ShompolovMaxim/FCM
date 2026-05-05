#pragma once
#include <QString>

struct PredictionParameters {
    QString algorithm = "const weights";
    bool useFuzzyValues = false;
    QString activationFunction = "bivalent";
    QString metric = "MSE";
    bool predictToStatic = false;
    double threshold = 0.0;
    int stepsLessThreshold = 1;
    int fixedSteps = 1;
    double fuzzinessDegree = 1.0;

    bool operator==(const PredictionParameters& other) const;
    bool operator!=(const PredictionParameters& other) const;
};

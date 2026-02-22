#pragma once
#include <QString>

struct PredictionParameters
{
    QString algorithm;
    QString activationFunction;
    QString metric;
    bool predictToStatic;
    double threshold;
    int stepsLessThreshold;
    int fixedSteps;
};

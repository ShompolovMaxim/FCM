#pragma once

#include "term.h"

#include <QString>

struct Weight {
    size_t id;
    QString name;
    QString description;
    double value;
    size_t fromConceptId;
    size_t toConceptId;
    std::vector<double> predictedValues;
};

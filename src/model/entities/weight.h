#pragma once

#include "term.h"

#include "map"

#include <QUuid>

struct Weight {
    QUuid id;
    QString name;
    QString description;
    std::shared_ptr<Term> term;
    QUuid fromConceptId;
    QUuid toConceptId;
    int dbId = -1;

    std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> predictedValues;
    std::map<double, double> sensitivity;
};

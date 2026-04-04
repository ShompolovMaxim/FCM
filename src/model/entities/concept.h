#pragma once

#include "term.h"

#include <map>

#include <QPointF>
#include <QUuid>

struct Concept {
    QUuid id;
    QString name;
    QString description;
    std::shared_ptr<Term> term;
    QPointF pos;
    size_t startStep;
    int dbId = -1;

    std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> predictedValues;
    std::map<double, double> sensitivity;
};

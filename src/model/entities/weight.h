#pragma once

#include "term.h"

#include "map"

#include <memory>
#include <variant>
#include <vector>
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

    bool operator==(const Weight& other) const;
    bool operator!=(const Weight& other) const;
};

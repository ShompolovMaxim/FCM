#pragma once

#include "concept_name_location.h"
#include "term.h"

#include <map>
#include <memory>
#include <variant>
#include <vector>

#include <QPointF>
#include <QUuid>

struct Concept {
    QUuid id;
    QString name;
    QString description;
    std::shared_ptr<Term> term;
    QPointF pos;
    size_t startStep;
    ConceptNameLocation nameLocation = ConceptNameLocation::Up;
    int dbId = -1;

    std::variant<std::vector<double>, std::vector<TriangularFuzzyValue>> predictedValues;
    std::map<double, double> sensitivity;

    bool operator==(const Concept& other) const;
    bool operator!=(const Concept& other) const;
};

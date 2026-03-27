#pragma once

#include "term.h"

#include <QPointF>
#include <QString>

struct Concept {
    size_t id;
    QString name;
    QString description;
    std::shared_ptr<Term> term;
    QPointF pos;
    size_t startStep;

    std::vector<double> predictedValues;
};

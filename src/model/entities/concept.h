#pragma once

#include <QPointF>
#include <QString>

struct Concept {
    size_t id;
    QString name;
    QString description;
    double value;
    QPointF pos;
    size_t startStep;
    std::vector<double> predictedValues;
};

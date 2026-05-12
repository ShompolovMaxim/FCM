#pragma once

#include "triangular_fuzzy_value.h"

#include "model/entities/element_type.h"

#include <QColor>
#include <QString>
#include <QUuid>

struct Term {
    QUuid id;
    QString name;
    QString description;
    double value = 0;
    TriangularFuzzyValue fuzzyValue;
    QColor color;
    ElementType type = ElementType::Node;
    int dbId = -1;

    bool operator==(const Term& other) const;
    bool operator!=(const Term& other) const;
};

#pragma once

#include "triangular_fuzzy_value.h"

#include "model/element_type.h"

#include <QColor>
#include <QString>

struct Term {
    size_t id;
    QString name;
    QString description;
    double value = 0;
    TriangularFuzzyValue fuzzyValue;
    QColor color;
    ElementType type = ElementType::Node;
};

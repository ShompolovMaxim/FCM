#pragma once

#include "model/entities/triangular_fuzzy_value.h"
#include "model/element_type.h"

#include <QColor>
#include <QString>

struct TemplateTerm {
    QString name;
    QString description;
    double value = 0;
    TriangularFuzzyValue fuzzyValue;
    QColor color;
    ElementType type = ElementType::Node;
};

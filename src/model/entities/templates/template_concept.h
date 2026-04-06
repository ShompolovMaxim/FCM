#pragma once

#include <cstddef>

#include <QPointF>
#include <QString>

struct TemplateConcept {
    QString name;
    QString description;
    QPointF pos;
    size_t startStep;
};

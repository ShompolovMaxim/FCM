#pragma once

#include "model/entities/concept_name_location.h"

#include <cstddef>

#include <QPointF>
#include <QString>

struct TemplateConcept {
    QString name;
    QString description;
    QPointF pos;
    size_t startStep;
    ConceptNameLocation nameLocation = ConceptNameLocation::Up;
};

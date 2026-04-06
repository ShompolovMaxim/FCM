#pragma once

#include "template_concept.h"

#include <memory>

#include <QString>

struct TemplateWeight {
    QString name;
    QString description;
    std::shared_ptr<TemplateConcept> fromConcept;
    std::shared_ptr<TemplateConcept> toConcept;
};

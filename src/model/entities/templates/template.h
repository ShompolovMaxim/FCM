#pragma once

#include "template_term.h"
#include "template_concept.h"
#include "template_weight.h"

#include <memory>
#include <vector>

#include <QString>

struct Template {
    QString name;
    QString description;
    std::vector<std::shared_ptr<TemplateTerm>> terms;
    std::vector<std::shared_ptr<TemplateConcept>> concepts;
    std::vector<std::shared_ptr<TemplateWeight>> weights;
};

#pragma once

#include "term.h"

#include <QString>

struct Weight {
    size_t id;
    QString name;
    QString description;
    size_t termId;
    size_t fromConceptId;
    size_t toConceptId;
};

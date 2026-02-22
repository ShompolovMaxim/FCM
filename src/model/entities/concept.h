#pragma once

#include "term.h"

#include <QString>

struct Concept {
    size_t id;
    QString name;
    QString description;
    size_t termId;
    size_t startStep;
};

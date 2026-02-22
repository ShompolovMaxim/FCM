#pragma once

#include "term.h"
#include "concept.h"
#include "weight.h"

#include <map>
#include <QString>

struct FCM {
    QString name;
    QString description;
    std::map<size_t, Term> terms;
    std::map<size_t, Concept> concepts;
    std::map<size_t, Weight> weights;

    size_t termsCounter = 0;
    size_t conceptsCounter = 0;
    size_t weightsCounter = 0;
};

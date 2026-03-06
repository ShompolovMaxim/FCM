#pragma once

#include "term.h"
#include "concept.h"
#include "weight.h"

#include "presenter/prediction_parameters.h"

#include <QDateTime>

struct Experiment {
    std::map<size_t, Term> terms;
    std::map<size_t, Concept> concepts;
    std::map<size_t, Weight> weights;
    PredictionParameters predictionParameters;
    QDateTime timestamp;
};

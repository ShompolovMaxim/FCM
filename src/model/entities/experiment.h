#pragma once

#include "term.h"
#include "concept.h"
#include "weight.h"

#include "presenter/prediction_parameters.h"

#include <QDateTime>

struct Experiment {
    std::map<QUuid, std::shared_ptr<Term>> terms;
    std::map<QUuid, std::shared_ptr<Concept>> concepts;
    std::map<QUuid, std::shared_ptr<Weight>> weights;
    PredictionParameters predictionParameters;
    QDateTime timestamp;
    int dbId = -1;
};

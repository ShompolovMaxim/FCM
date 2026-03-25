#pragma once

#include "concept.h"
#include "experiment.h"
#include "term.h"
#include "weight.h"

#include <map>
#include <QString>

struct FCM {
    QString name;
    QString description;
    std::map<size_t, std::shared_ptr<Term>> terms;
    std::map<size_t, std::shared_ptr<Concept>> concepts;
    std::map<size_t, std::shared_ptr<Weight>> weights;
    PredictionParameters predictionParameters;
    std::vector<Experiment> experiments;

    size_t termsCounter = 0;
    size_t conceptsCounter = 0;
    size_t weightsCounter = 0;
};

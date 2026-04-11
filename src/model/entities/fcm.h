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
    std::map<QUuid, std::shared_ptr<Term>> terms;
    std::map<QUuid, std::shared_ptr<Concept>> concepts;
    std::map<QUuid, std::shared_ptr<Weight>> weights;
    PredictionParameters predictionParameters;
    std::vector<Experiment> experiments;
    bool autosaveOn = false;
    int dbId = -1;

    QList<int> deletedTermsIds;
    QList<int> deletedConceptsIds;
    QList<int> deletedWeightsIds;
    QList<int> deletedExperimentsIds;
};

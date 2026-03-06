#ifndef MODELS_REPOSITORY_H
#define MODELS_REPOSITORY_H

#include "model/entities/fcm.h"

#include <QList>
#include <QString>
#include <QSqlDatabase>

class ModelsRepository
{
public:
    ModelsRepository(QSqlDatabase database);

    std::optional<size_t> createConcept(const Concept& concept, const size_t experimentId);
    std::optional<size_t> createWeight(const Weight& weight, const size_t experimentId, const size_t fromConceptId, const size_t toConceptId);
    std::optional<size_t> createTerm(const Term& term, const size_t experimentId);
    std::optional<size_t> createExperiment(const Experiment& experiment, const size_t modelId);
    std::optional<size_t> createModel(const FCM& fcm);

    std::optional<std::vector<Concept>> getExperimentConcepts(const size_t experimentId);
    std::optional<std::vector<Weight>> getExperimentWeights(const size_t experimentId);
    std::optional<std::vector<Term>> getExperimentTerms(const size_t experimentId);
    std::optional<std::vector<Experiment>> getExperiments(const size_t modelId);
    QList<QString> getModelsNames();
    std::optional<FCM> getModel(const QString modelName);
private:
    QSqlDatabase db;
};

#endif // MODELS_REPOSITORY_H

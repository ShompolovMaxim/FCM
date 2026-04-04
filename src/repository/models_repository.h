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

    std::optional<int> createConcept(const Concept& conceptConst, const int experimentId, const std::optional<int>& dbTermId);
    std::optional<int> createWeight(const Weight& weightConst, const int experimentId, const int fromConceptId, const int toConceptId, const std::map<QUuid, int>& termsDBIds);
    std::optional<int> createTerm(const Term& termConst, const int experimentId);
    std::optional<int> createExperiment(const Experiment& experimentConst, const int modelId);
    std::optional<int> createModel(const FCM& fcm);

    std::optional<std::vector<Concept>> getExperimentConcepts(const int experimentId, const std::map<QUuid, std::shared_ptr<Term>>& terms);
    std::optional<std::vector<Weight>> getExperimentWeights(const int experimentId, const std::map<QUuid, std::shared_ptr<Term>>& terms, const std::map<int, std::shared_ptr<Concept>>& conceptsByDbId);
    std::optional<std::map<QUuid, std::shared_ptr<Term>>> getExperimentTerms(const int experimentId);
    std::optional<std::vector<Experiment>> getExperiments(const int modelId);
    QList<QString> getModelsNames();
    std::optional<FCM> getModel(const QString modelName);
private:
    QSqlDatabase db;
};

#endif // MODELS_REPOSITORY_H

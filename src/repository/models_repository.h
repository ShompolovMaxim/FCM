#pragma once

#include "model/entities/fcm.h"

#include <optional>
#include <vector>
#include <map>
#include <utility>

#include <QString>
#include <QSqlDatabase>
#include <QList>
#include <QDateTime>

class ModelsRepository {
public:
    explicit ModelsRepository(QSqlDatabase database);

    bool transaction();
    bool commit();
    bool rollback();

    QList<QString> getModelsNames();

    std::optional<FCM> getModel(const QString& modelName);
    std::optional<int> createModel(FCM& fcm);
    bool updateModel(const FCM& fcm);
    bool deleteModel(int modelId);

    std::optional<std::vector<Experiment>> getExperiments(int modelId);
    std::optional<std::vector<std::pair<int, QDateTime>>> getExperimentsInfo(int modelId);
    std::optional<int> createExperiment(Experiment& experiment, int modelId);
    bool updateExperiment(const Experiment& experiment);
    bool deleteExperiment(int experimentId);

    std::optional<std::map<QUuid,std::shared_ptr<Term>>> getExperimentTerms(int experimentId);
    std::optional<int> createTerm(Term& term, int experimentId);
    bool updateTerm(const Term& term);
    bool deleteTerm(int termId);

    std::optional<std::vector<Concept>> getExperimentConcepts(int experimentId, const std::map<QUuid,std::shared_ptr<Term>>& terms);
    std::optional<int> createConcept(Concept& concept, int experimentId, const std::optional<int>& dbTermId);
    bool updateConcept(const Concept& concept);
    bool deleteConcept(int conceptId);

    std::optional<std::vector<Weight>> getExperimentWeights(int experimentId, const std::map<QUuid,std::shared_ptr<Term>>& terms, const std::map<int,std::shared_ptr<Concept>>& conceptsByDbId);
    std::optional<int> createWeight(Weight& weight, int experimentId, int fromConceptId, int toConceptId, const std::map<QUuid,int>& termsDBIds);
    bool updateWeight(const Weight& weight);
    bool deleteWeight(int weightId);

private:
    QSqlDatabase db;
};

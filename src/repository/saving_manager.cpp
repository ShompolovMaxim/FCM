#include "saving_manager.h"

#include <QDateTime>

SavingManager::SavingManager(ModelsRepository repository) : repo(repository) {}

bool SavingManager::saveAs(FCM &fcm) {
    if (!repo.transaction())
        return false;

    resetFCMDbIds(fcm);

    auto modelIdOpt = repo.createModel(fcm);
    if (!modelIdOpt) {
        repo.rollback();
        return false;
    }

    fcm.dbId = *modelIdOpt;

    if (!saveExperiments(fcm) || !saveCurrentExperiment(fcm, false)) {
        repo.rollback();
        return false;
    }

    if (!repo.commit()) {
        repo.rollback();
        return false;
    }

    return true;
}

bool SavingManager::saveFCM(FCM &fcm) {
    if (!repo.transaction())
        return false;

    if (fcm.dbId == -1) {
        auto modelIdOpt = repo.createModel(fcm);
        if (!modelIdOpt) {
            repo.rollback();
            return false;
        }
        fcm.dbId = *modelIdOpt;
    } else if (!repo.updateModel(fcm)) {
        repo.rollback();
        return false;
    }

    if (fcm.dbId != -1 && currentExperimentIds.find(fcm.dbId) == currentExperimentIds.end()) {
        auto currentExperimentId = getCurrentExperimentId(fcm);
        if (!currentExperimentId.has_value()) {
            repo.rollback();
            return false;
        }
    }

    if (!saveExperiments(fcm) || !saveCurrentExperiment(fcm, true)) {
        repo.rollback();
        return false;
    }

    if (!repo.commit()) {
        repo.rollback();
        return false;
    }

    if (auto currentExperimentId = getCurrentExperimentId(fcm); currentExperimentId.has_value())
        currentExperimentIds[fcm.dbId] = *currentExperimentId;

    fcm.deletedTermsIds.clear();
    fcm.deletedConceptsIds.clear();
    fcm.deletedWeightsIds.clear();
    return true;
}

bool SavingManager::deleteFCM(int fcmDbId) {
    if (!repo.transaction())
        return false;

    auto experimentsOpt = repo.getExperimentsInfo(fcmDbId);
    if (!experimentsOpt) {
        repo.rollback();
        return false;
    }

    for (const auto &[experimentId, _] : *experimentsOpt) {
        if (!deleteExperimentElements(experimentId) || !repo.deleteExperiment(experimentId)) {
            repo.rollback();
            return false;
        }
    }

    if (!repo.deleteModel(fcmDbId)) {
        repo.rollback();
        return false;
    }

    currentExperimentIds.erase(fcmDbId);

    if (!repo.commit()) {
        repo.rollback();
        return false;
    }

    return true;
}

std::optional<FCM> SavingManager::getFCM(const QString &modelName) {
    auto fcmOpt = repo.getModel(modelName);
    if (!fcmOpt.has_value())
        return {};

    auto currentExperimentId = getCurrentExperimentId(*fcmOpt);
    if (!currentExperimentId.has_value())
        return {};

    currentExperimentIds[fcmOpt->dbId] = *currentExperimentId;
    return fcmOpt;
}

QList<QString> SavingManager::getModelsNames() {
    return repo.getModelsNames();
}

bool SavingManager::saveExperiments(FCM &fcm) {
    for (auto &exp : fcm.experiments) {
        if (!saveExperiment(exp, fcm.dbId))
            return false;
    }
    return true;
}

bool SavingManager::saveCurrentExperiment(FCM &fcm, bool applyDeletedIds) {
    Experiment current;
    current.terms = fcm.terms;
    current.concepts = fcm.concepts;
    current.weights = fcm.weights;
    current.predictionParameters = fcm.predictionParameters;
    current.timestamp = QDateTime::currentDateTime();

    if (applyDeletedIds) {
        auto currentExperimentId = getCurrentExperimentId(fcm);
        if (!currentExperimentId.has_value())
            return false;
        current.dbId = *currentExperimentId;
        return saveExperiment(current, fcm.dbId, &fcm);
    }

    if (!saveExperiment(current, fcm.dbId))
        return false;

    currentExperimentIds[fcm.dbId] = current.dbId;
    return true;
}

bool SavingManager::saveExperiment(Experiment &exp, int modelId, const FCM *deletedElementsSource) {
    if (exp.dbId == -1) {
        auto expIdOpt = repo.createExperiment(exp, modelId);
        if (!expIdOpt)
            return false;
        exp.dbId = *expIdOpt;
    } else {
        if (deletedElementsSource && !deleteCurrentElements(*deletedElementsSource))
            return false;
        if (!repo.updateExperiment(exp))
            return false;
    }

    if (!saveTerms(exp, exp.dbId) || !saveConcepts(exp, exp.dbId) || !saveWeights(exp, exp.dbId))
        return false;

    return true;
}

bool SavingManager::saveTerms(Experiment &exp, int experimentId) {
    for (const auto &[termId, termPtr] : exp.terms) {
        Q_UNUSED(termId)
        if (termPtr->dbId == -1) {
            auto idOpt = repo.createTerm(*termPtr, experimentId);
            if (!idOpt)
                return false;
            termPtr->dbId = *idOpt;
        } else if (!repo.updateTerm(*termPtr)) {
            return false;
        }
    }
    return true;
}

bool SavingManager::saveConcepts(Experiment &exp, int experimentId) {
    for (const auto &[conceptId, conceptPtr] : exp.concepts) {
        Q_UNUSED(conceptId)
        std::optional<int> dbTermId;
        if (conceptPtr->term)
            dbTermId = conceptPtr->term->dbId;

        if (conceptPtr->dbId == -1) {
            auto idOpt = repo.createConcept(*conceptPtr, experimentId, dbTermId);
            if (!idOpt)
                return false;
            conceptPtr->dbId = *idOpt;
        } else if (!repo.updateConcept(*conceptPtr)) {
            return false;
        }
    }
    return true;
}

bool SavingManager::saveWeights(Experiment &exp, int experimentId) {
    std::map<QUuid, int> termsDbIds;
    std::map<QUuid, int> conceptsDbIds;

    for (const auto &[termId, termPtr] : exp.terms)
        termsDbIds[termId] = termPtr->dbId;

    for (const auto &[conceptId, conceptPtr] : exp.concepts)
        conceptsDbIds[conceptId] = conceptPtr->dbId;

    for (const auto &[weightId, weightPtr] : exp.weights) {
        Q_UNUSED(weightId)
        auto itFrom = conceptsDbIds.find(weightPtr->fromConceptId);
        auto itTo = conceptsDbIds.find(weightPtr->toConceptId);

        if (itFrom == conceptsDbIds.end() || itTo == conceptsDbIds.end())
            return false;

        int fromDb = itFrom->second;
        int toDb = itTo->second;

        if (weightPtr->dbId == -1) {
            auto idOpt = repo.createWeight(*weightPtr, experimentId, fromDb, toDb, termsDbIds);
            if (!idOpt)
                return false;
            weightPtr->dbId = *idOpt;
        } else if (!repo.updateWeight(*weightPtr)) {
            return false;
        }
    }
    return true;
}

bool SavingManager::deleteCurrentElements(const FCM &fcm) {
    for (int weightId : fcm.deletedWeightsIds) {
        if (!repo.deleteWeight(weightId))
            return false;
    }

    for (int conceptId : fcm.deletedConceptsIds) {
        if (!repo.deleteConcept(conceptId))
            return false;
    }

    for (int termId : fcm.deletedTermsIds) {
        if (!repo.deleteTerm(termId))
            return false;
    }

    return true;
}

bool SavingManager::deleteExperimentElements(int experimentId) {
    auto termsOpt = repo.getExperimentTerms(experimentId);
    if (!termsOpt)
        return false;

    auto conceptsOpt = repo.getExperimentConcepts(experimentId, *termsOpt);
    if (!conceptsOpt)
        return false;

    std::map<int, std::shared_ptr<Concept>> conceptsByDbId;
    for (const auto &concept : *conceptsOpt)
        conceptsByDbId[concept.dbId] = std::make_shared<Concept>(concept);

    auto weightsOpt = repo.getExperimentWeights(experimentId, *termsOpt, conceptsByDbId);
    if (!weightsOpt)
        return false;

    for (const auto &weight : *weightsOpt) {
        if (!repo.deleteWeight(weight.dbId))
            return false;
    }

    for (const auto &concept : *conceptsOpt) {
        if (!repo.deleteConcept(concept.dbId))
            return false;
    }

    for (const auto &[_, term] : *termsOpt) {
        if (!repo.deleteTerm(term->dbId))
            return false;
    }

    return true;
}

std::optional<int> SavingManager::getCurrentExperimentId(const FCM &fcm) {
    auto it = currentExperimentIds.find(fcm.dbId);
    if (it != currentExperimentIds.end())
        return it->second;

    auto experimentsOpt = repo.getExperimentsInfo(fcm.dbId);
    if (!experimentsOpt)
        return {};

    std::optional<std::pair<int, QDateTime>> currentExperiment;
    for (const auto &[experimentId, timestamp] : *experimentsOpt) {
        if (!currentExperiment.has_value() || currentExperiment->second < timestamp)
            currentExperiment = {experimentId, timestamp};
    }

    if (!currentExperiment.has_value())
        return {};

    currentExperimentIds[fcm.dbId] = currentExperiment->first;
    return currentExperiment->first;
}

void SavingManager::resetFCMDbIds(FCM &fcm) {
    fcm.dbId = -1;
    fcm.deletedTermsIds.clear();
    fcm.deletedConceptsIds.clear();
    fcm.deletedWeightsIds.clear();

    for (const auto &[termId, termPtr] : fcm.terms) {
        termPtr->dbId = -1;
    }

    for (const auto &[conceptId, conceptPtr] : fcm.concepts) {
        conceptPtr->dbId = -1;
    }

    for (const auto &[weightId, weightPtr] : fcm.weights) {
        weightPtr->dbId = -1;
    }

    for (auto &exp : fcm.experiments) {
        exp.dbId = -1;
        for (const auto &[termId, termPtr] : exp.terms) {
            termPtr->dbId = -1;
        }
        for (const auto &[conceptId, conceptPtr] : exp.concepts) {
            conceptPtr->dbId = -1;
        }
        for (const auto &[weightId, weightPtr] : exp.weights) {
            weightPtr->dbId = -1;
        }
    }
}

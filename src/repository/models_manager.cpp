#include "models_manager.h"

#include "common/logger.h"

#include <QDateTime>

ModelsSavingManager::ModelsSavingManager(ModelsRepository repository) : repo(repository) {}

bool ModelsSavingManager::saveAs(FCM &fcm) {
    if (!repo.transaction()) {
        Logger::warn("Save transaction failed");
        return false;
    }

    resetFCMDbIds(fcm);

    auto modelIdOpt = repo.createModel(fcm);
    if (!modelIdOpt) {
        Logger::warn("Save model create failed");
        repo.rollback();
        return false;
    }

    fcm.dbId = *modelIdOpt;

    if (!saveExperiments(fcm) || !saveCurrentExperiment(fcm, false)) {
        Logger::warn("Save data failed");
        repo.rollback();
        return false;
    }

    if (!repo.commit()) {
        Logger::warn("Save commit failed");
        repo.rollback();
        return false;
    }

    return true;
}

bool ModelsSavingManager::saveFCM(FCM &fcm) {
    if (!repo.transaction()) {
        Logger::warn("Save transaction failed");
        return false;
    }

    if (fcm.dbId == -1) {
        auto modelIdOpt = repo.createModel(fcm);
        if (!modelIdOpt) {
            Logger::warn("Save model create failed");
            repo.rollback();
            return false;
        }
        fcm.dbId = *modelIdOpt;
    } else if (!repo.updateModel(fcm)) {
        Logger::warn("Save model update failed");
        repo.rollback();
        return false;
    }

    if (fcm.dbId != -1 && currentExperimentIds.find(fcm.dbId) == currentExperimentIds.end()) {
        auto currentExperimentId = getCurrentExperimentId(fcm);
        if (!currentExperimentId.has_value()) {
            Logger::warn("Current experiment missing");
            repo.rollback();
            return false;
        }
    }

    if (!deleteExperiments(fcm) || !saveExperiments(fcm) || !saveCurrentExperiment(fcm, true)) {
        Logger::warn("Save sync failed");
        repo.rollback();
        return false;
    }

    if (!repo.commit()) {
        Logger::warn("Save commit failed");
        repo.rollback();
        return false;
    }

    if (auto currentExperimentId = getCurrentExperimentId(fcm); currentExperimentId.has_value()) {
        currentExperimentIds[fcm.dbId] = *currentExperimentId;
    }

    fcm.deletedTermsIds.clear();
    fcm.deletedConceptsIds.clear();
    fcm.deletedWeightsIds.clear();
    fcm.deletedExperimentsIds.clear();
    return true;
}

bool ModelsSavingManager::deleteFCM(int fcmDbId) {
    if (!repo.transaction()) {
        Logger::warn("Delete transaction failed");
        return false;
    }

    auto experimentsOpt = repo.getExperimentsInfo(fcmDbId);
    if (!experimentsOpt) {
        Logger::warn("Delete experiments load failed");
        repo.rollback();
        return false;
    }

    for (const auto &[experimentId, _] : *experimentsOpt) {
        if (!deleteExperimentElements(experimentId) || !repo.deleteExperiment(experimentId)) {
            Logger::warn("Delete experiment failed");
            repo.rollback();
            return false;
        }
    }

    if (!repo.deleteModel(fcmDbId)) {
        Logger::warn("Delete model failed");
        repo.rollback();
        return false;
    }

    currentExperimentIds.erase(fcmDbId);

    if (!repo.commit()) {
        Logger::warn("Delete commit failed");
        repo.rollback();
        return false;
    }

    return true;
}

std::optional<FCM> ModelsSavingManager::getFCM(const QString &modelName) {
    auto fcmOpt = repo.getModel(modelName);
    if (!fcmOpt.has_value()) {
        Logger::warn("Load model failed");
        return {};
    }

    auto currentExperimentId = getCurrentExperimentId(*fcmOpt);
    if (!currentExperimentId.has_value()) {
        Logger::warn("Current experiment missing");
        return {};
    }

    currentExperimentIds[fcmOpt->dbId] = *currentExperimentId;
    return fcmOpt;
}

std::optional<FCM> ModelsSavingManager::getFCM(int modelId) {
    auto fcmOpt = repo.getModel(modelId);
    if (!fcmOpt.has_value()) {
        Logger::warn("Load model failed");
        return {};
    }

    auto currentExperimentId = getCurrentExperimentId(*fcmOpt);
    if (!currentExperimentId.has_value()) {
        Logger::warn("Current experiment missing");
        return {};
    }

    currentExperimentIds[fcmOpt->dbId] = *currentExperimentId;
    return fcmOpt;
}

QList<QString> ModelsSavingManager::getModelsNames() {
    return repo.getModelsNames();
}

bool ModelsSavingManager::saveExperiments(FCM &fcm) {
    for (auto &exp : fcm.experiments) {
        if (!saveExperiment(exp, fcm.dbId)) {
            return false;
        }
    }
    return true;
}

bool ModelsSavingManager::saveCurrentExperiment(FCM &fcm, bool applyDeletedIds) {
    Experiment current;
    current.terms = fcm.terms;
    current.concepts = fcm.concepts;
    current.weights = fcm.weights;
    current.predictionParameters = fcm.predictionParameters;
    current.timestamp = QDateTime::currentDateTime();

    if (applyDeletedIds) {
        auto currentExperimentId = getCurrentExperimentId(fcm);
        if (!currentExperimentId.has_value()) {
            Logger::warn("Current experiment missing");
            return false;
        }
        current.dbId = *currentExperimentId;
        return saveExperiment(current, fcm.dbId, &fcm);
    }

    if (!saveExperiment(current, fcm.dbId)) {
        return false;
    }

    currentExperimentIds[fcm.dbId] = current.dbId;
    return true;
}

bool ModelsSavingManager::saveExperiment(Experiment &exp, int modelId, const FCM *deletedElementsSource) {
    if (exp.dbId == -1) {
        auto expIdOpt = repo.createExperiment(exp, modelId);
        if (!expIdOpt) {
            Logger::warn("Experiment create failed");
            return false;
        }
        exp.dbId = *expIdOpt;
    } else {
        if (deletedElementsSource && !deleteCurrentElements(*deletedElementsSource)) {
            Logger::warn("Current elements delete failed");
            return false;
        }
        if (!repo.updateExperiment(exp)) {
            Logger::warn("Experiment update failed");
            return false;
        }
    }

    if (!saveTerms(exp, exp.dbId) || !saveConcepts(exp, exp.dbId) || !saveWeights(exp, exp.dbId)) {
        Logger::warn("Experiment save failed");
        return false;
    }

    return true;
}

bool ModelsSavingManager::saveTerms(Experiment &exp, int experimentId) {
    for (const auto &[termId, termPtr] : exp.terms) {
        if (!termPtr) {
            Logger::warn("Term pointer missing");
            return false;
        }
        if (termPtr->dbId == -1) {
            auto idOpt = repo.createTerm(*termPtr, experimentId);
            if (!idOpt) {
                Logger::warn("Term create failed");
                return false;
            }
            termPtr->dbId = *idOpt;
        } else if (!repo.updateTerm(*termPtr)) {
            Logger::warn("Term update failed");
            return false;
        }
    }
    return true;
}

bool ModelsSavingManager::saveConcepts(Experiment &exp, int experimentId) {
    for (const auto &[conceptId, conceptPtr] : exp.concepts) {
        if (!conceptPtr) {
            Logger::warn("Concept pointer missing");
            return false;
        }
        std::optional<int> dbTermId;
        if (conceptPtr->term) {
            dbTermId = conceptPtr->term->dbId;
        }

        if (conceptPtr->dbId == -1) {
            auto idOpt = repo.createConcept(*conceptPtr, experimentId, dbTermId);
            if (!idOpt) {
                Logger::warn("Concept create failed");
                return false;
            }
            conceptPtr->dbId = *idOpt;
        } else if (!repo.updateConcept(*conceptPtr)) {
            Logger::warn("Concept update failed");
            return false;
        }
    }
    return true;
}

bool ModelsSavingManager::saveWeights(Experiment &exp, int experimentId) {
    std::map<QUuid, int> termsDbIds;
    std::map<QUuid, int> conceptsDbIds;

    for (const auto &[termId, termPtr] : exp.terms) {
        termsDbIds[termId] = termPtr->dbId;
    }

    for (const auto &[conceptId, conceptPtr] : exp.concepts) {
        conceptsDbIds[conceptId] = conceptPtr->dbId;
    }

    for (const auto &[weightId, weightPtr] : exp.weights) {
        if (!weightPtr) {
            Logger::warn("Weight pointer missing");
            return false;
        }
        auto itFrom = conceptsDbIds.find(weightPtr->fromConceptId);
        auto itTo = conceptsDbIds.find(weightPtr->toConceptId);

        if (itFrom == conceptsDbIds.end() || itTo == conceptsDbIds.end()) {
            Logger::warn("Weight concepts missing");
            return false;
        }

        int fromDb = itFrom->second;
        int toDb = itTo->second;

        if (weightPtr->dbId == -1) {
            auto idOpt = repo.createWeight(*weightPtr, experimentId, fromDb, toDb, termsDbIds);
            if (!idOpt) {
                Logger::warn("Weight create failed");
                return false;
            }
            weightPtr->dbId = *idOpt;
        } else if (!repo.updateWeight(*weightPtr)) {
            Logger::warn("Weight update failed");
            return false;
        }
    }
    return true;
}

bool ModelsSavingManager::deleteExperiments(FCM &fcm) {
    if (fcm.deletedExperimentsIds.empty()) {
        return true;
    }

    auto currentExperimentId = getCurrentExperimentId(fcm);
    if (!currentExperimentId.has_value()) {
        Logger::warn("Current experiment missing");
        return false;
    }

    for (int experimentId : fcm.deletedExperimentsIds) {
        if (experimentId == *currentExperimentId) {
            Logger::warn("Current experiment delete blocked");
            return false;
        }
        if (!deleteExperimentElements(experimentId) || !repo.deleteExperiment(experimentId)) {
            Logger::warn("Experiment delete failed");
            return false;
        }
    }

    return true;
}

bool ModelsSavingManager::deleteCurrentElements(const FCM &fcm) {
    for (int weightId : fcm.deletedWeightsIds) {
        if (!repo.deleteWeight(weightId)) {
            Logger::warn("Weight delete failed");
            return false;
        }
    }

    for (int conceptId : fcm.deletedConceptsIds) {
        if (!repo.deleteConcept(conceptId)) {
            Logger::warn("Concept delete failed");
            return false;
        }
    }

    for (int termId : fcm.deletedTermsIds) {
        if (!repo.deleteTerm(termId)) {
            Logger::warn("Term delete failed");
            return false;
        }
    }

    return true;
}

bool ModelsSavingManager::deleteExperimentElements(int experimentId) {
    auto termsOpt = repo.getExperimentTerms(experimentId);
    if (!termsOpt) {
        Logger::warn("Experiment terms load failed");
        return false;
    }

    auto conceptsOpt = repo.getExperimentConcepts(experimentId, *termsOpt);
    if (!conceptsOpt) {
        Logger::warn("Experiment concepts load failed");
        return false;
    }

    std::map<int, std::shared_ptr<Concept>> conceptsByDbId;
    for (const auto &concept : *conceptsOpt) {
        conceptsByDbId[concept.dbId] = std::make_shared<Concept>(concept);
    }

    auto weightsOpt = repo.getExperimentWeights(experimentId, *termsOpt, conceptsByDbId);
    if (!weightsOpt) {
        Logger::warn("Experiment weights load failed");
        return false;
    }

    for (const auto &weight : *weightsOpt) {
        if (!repo.deleteWeight(weight.dbId)) {
            Logger::warn("Weight delete failed");
            return false;
        }
    }

    for (const auto &concept : *conceptsOpt) {
        if (!repo.deleteConcept(concept.dbId)) {
            Logger::warn("Concept delete failed");
            return false;
        }
    }

    for (const auto &[_, term] : *termsOpt) {
        if (!repo.deleteTerm(term->dbId)) {
            Logger::warn("Term delete failed");
            return false;
        }
    }

    return true;
}

std::optional<int> ModelsSavingManager::getCurrentExperimentId(const FCM &fcm) {
    auto it = currentExperimentIds.find(fcm.dbId);
    if (it != currentExperimentIds.end()) {
        return it->second;
    }

    auto experimentsOpt = repo.getExperimentsInfo(fcm.dbId);
    if (!experimentsOpt) {
        Logger::warn("Experiments info load failed");
        return {};
    }

    std::optional<std::pair<int, QDateTime>> currentExperiment;
    for (const auto &[experimentId, timestamp] : *experimentsOpt) {
        if (!currentExperiment.has_value() || currentExperiment->second < timestamp) {
            currentExperiment = {experimentId, timestamp};
        }
    }

    if (!currentExperiment.has_value()) {
        Logger::warn("Current experiment missing");
        return {};
    }

    currentExperimentIds[fcm.dbId] = currentExperiment->first;
    return currentExperiment->first;
}

void ModelsSavingManager::resetFCMDbIds(FCM &fcm) {
    fcm.dbId = -1;
    fcm.deletedTermsIds.clear();
    fcm.deletedConceptsIds.clear();
    fcm.deletedWeightsIds.clear();
    fcm.deletedExperimentsIds.clear();

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

